"""Prepare and test an isolated Astronaut greeter. Never change system SDDM configuration."""
import argparse
import configparser
import copy
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

MEDIA = {'.png', '.jpg', '.jpeg', '.webp', '.gif', '.avi', '.mp4', '.mov', '.mkv', '.m4v', '.webm'}

def ini(path):
    parser = configparser.ConfigParser(interpolation=None, strict=False)
    parser.optionxform = str
    parser.read(path)
    return parser

def value(section, key, default=''):
    return section.get(key, default).strip('"')

def atomic_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(data, indent=2) + '\n')
    temporary.replace(path)

def contained(root, name):
    path = (root / name).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError('Media path escapes its project')
    return path

def system_theme():
    directory = '/usr/share/sddm/themes'
    files = sorted(Path('/usr/lib/sddm/sddm.conf.d').glob('*.conf')) + sorted(Path('/etc/sddm.conf.d').glob('*.conf')) + [Path('/etc/sddm.conf')]
    for file in files:
        config = ini(file)
        if config.has_option('Theme', 'ThemeDir'):
            directory = value(config['Theme'], 'ThemeDir', directory)
    return Path(directory) / 'sddm-astronaut-theme'

def catalog(roots, theme):
    entries = []
    for media in sorted((theme / 'Backgrounds').glob('*')):
        if media.is_file() and media.suffix.lower() in MEDIA:
            entries.append(dict(id=str(media.resolve()), label='SDDM · ' + media.stem, media=str(media.resolve()), supported=True))
    seen = set()
    for root in roots:
        for project in sorted(Path(root).glob('*/project.json')):
            if str(project.resolve()) in seen:
                continue
            seen.add(str(project.resolve()))
            try:
                data = json.loads(project.read_text())
                kind = data.get('type', '').lower()
                name = data.get('lab_media_file') or (data.get('file') if kind == 'video' else '')
                media = contained(project.parent, name) if name else None
                supported = bool(media and media.is_file() and media.suffix.lower() in MEDIA)
                origin = 'Local' if data.get('lab_origin') == 'local' else 'Workshop'
                entries.append(dict(id=str(project.parent.resolve()), label=origin + ' · ' + str(data.get('title', project.parent.name)) + ((' · ' + kind) if not supported else ''),
                                    media=str(media) if supported else '', supported=supported, kind=kind))
            except (ValueError, OSError, TypeError, AttributeError):
                continue
    return entries

class Integration:
    def __init__(self, request):
        self.request = request
        self.config_file = Path(request['config']) / 'sddm.json'
        self.state = Path(request['state']) / 'sddm'
        self.state.mkdir(parents=True, exist_ok=True)
        self.template = Path(__file__).resolve().parent / 'sddm-astronaut'
        if not self.template.exists():
            self.template = Path(__file__).resolve().parents[1] / 'vendor/sddm-astronaut-theme'
        # The system theme supplies its own wallpapers; shipped templates contain code, icons and notices only.
        self.theme = Path(request['theme']) if 'theme' in request else system_theme()
        self.config = json.loads(self.config_file.read_text()) if self.config_file.exists() else dict(enabled=False, wallpaper={}, ui={'preset': 'astronaut'})
        self.entries = catalog(request.get('roots', []), self.theme)
        self.presets = [p.stem for p in sorted((self.template / 'Themes').glob('*.conf'))]
        self.greeter = shutil.which('sddm-greeter-qt6')
        self.bundle = self.state / 'prepared'

    def fingerprint(self, config):
        h = hashlib.sha256(json.dumps(config, sort_keys=True).encode())
        for root in (self.template, self.bundle):
            for path in sorted(root.rglob('*')):
                if path.is_file() and '.git' not in path.parts:
                    stat = path.stat()
                    h.update(f'{path}:{stat.st_size}:{stat.st_mtime_ns}'.encode())
        if self.greeter:
            stat = Path(self.greeter).stat()
            h.update(f'{self.greeter}:{stat.st_size}:{stat.st_mtime_ns}'.encode())
        for base in ('/usr/lib', '/usr/lib64'):
            for name in ('libQt6Core.so.6', 'libQt6Qml.so.6', 'libQt6Multimedia.so.6'):
                path = Path(base)/name
                if path.exists():
                    stat = path.stat()
                    h.update(f'{path}:{stat.st_size}:{stat.st_mtime_ns}'.encode())
        return h.hexdigest()

    def ready(self):
        try:
            receipt = json.loads((self.state / 'verified.json').read_text())
            return bool(self.config['enabled'] and self.greeter and self.theme.is_dir() and (self.bundle/'Main.qml').is_file()
                        and receipt['fingerprint'] == self.fingerprint(self.config))
        except (OSError, ValueError, KeyError):
            return False

    def prepare(self, candidate):
        if not self.greeter or not self.presets or not self.theme.is_dir():
            raise ValueError('Install SDDM Qt6 and the Astronaut theme first')
        selected = candidate['wallpaper'].get('source')
        if not selected:
            default = value(ini(self.theme/'Themes/astronaut.conf')['General'], 'Background')
            selected = str((self.theme/default).resolve())
            candidate['wallpaper']['source'] = selected
        entry = next((row for row in self.entries if row['id'] == selected), None)
        if not entry or not entry['supported']:
            raise ValueError('Choose an image, GIF or video. Engine scenes and web projects need a future greeter renderer bridge')
        source = Path(entry['media'])
        # Probe actual media, never substitute a Workshop thumbnail for a scene.
        probe = subprocess.run(['ffprobe', '-v', 'error', '-protocol_whitelist', 'file', '-select_streams', 'v:0',
                                '-show_entries', 'stream=width,height', '-of', 'json', str(source)], capture_output=True, text=True, timeout=20)
        streams = json.loads(probe.stdout).get('streams', [])
        if probe.returncode or not streams or streams[0].get('width', 0) < 1:
            raise ValueError('Cannot decode selected media')
        preset = candidate['ui']['preset']
        if preset not in self.presets:
            raise ValueError('Unknown login UI preset')
        stage = Path(tempfile.mkdtemp(prefix='stage-', dir=self.state))
        try:
            for name in ('Components', 'Assets'):
                shutil.copytree(self.template/name, stage/name)
            if (self.theme/'Fonts').is_dir():
                shutil.copytree(self.theme/'Fonts', stage/'Fonts')
            for name in ('LICENSE', 'metadata.desktop'):
                shutil.copy2(self.template/name, stage/name)
            shutil.copy2(self.template/'Main.qml', stage/'ThemeMain.qml')
            (stage/'Main.qml').write_text('import QtQuick\nimport "."\nThemeMain { Component.onCompleted: console.log("WALLPAPER_LAB_GREETER_READY") }\n')
            metadata = ini(stage/'metadata.desktop')
            metadata['SddmGreeterTheme']['ConfigFile'] = 'theme.conf'
            with (stage/'metadata.desktop').open('w') as out:
                metadata.write(out)
            background = stage/'Backgrounds'/('wallpaper' + source.suffix.lower())
            background.parent.mkdir()
            shutil.copyfile(source, background)
            config = ini(self.template/'Themes'/(preset+'.conf'))
            general = config['General']
            # Wallpaper preferences remain independent of UI preset defaults.
            general['Background'] = 'Backgrounds/' + background.name
            general['background'] = general['Background']
            general['BackgroundPlaceholder'] = ''
            general['ScreenWidth'] = ''
            general['ScreenHeight'] = ''
            general['CropBackground'] = 'true' if candidate['wallpaper'].get('crop', True) else 'false'
            general['PauseBackground'] = 'false'
            general['BackgroundSpeed'] = '1.0'
            position = candidate['ui'].get('position', 'preset')
            if position != 'preset':
                if position not in ('left', 'center', 'right'):
                    raise ValueError('Invalid login form position')
                general['FormPosition'] = position
            if 'blur' in candidate['ui']:
                general['PartialBlur'] = 'true' if candidate['ui']['blur'] else 'false'
                general['FullBlur'] = 'false'
            with (stage/'theme.conf').open('w') as out:
                config.write(out)
            stage.chmod(0o755)
            for path in stage.rglob('*'):
                path.chmod(0o755 if path.is_dir() else 0o644)
            self.check(stage)
            previous = self.state/'previous'
            if previous.exists():
                shutil.rmtree(previous)
            if self.bundle.exists():
                self.bundle.rename(previous)
            try:
                stage.rename(self.bundle)
            except OSError:
                if previous.exists():
                    previous.rename(self.bundle)
                raise
            return candidate
        finally:
            if stage.exists():
                shutil.rmtree(stage)

    def check(self, theme):
        env = os.environ.copy()
        env.update(QT_QPA_PLATFORM='offscreen', QT_QUICK_BACKEND='software', QT_FORCE_STDERR_LOGGING='1', QT_ASSUME_STDERR_HAS_CONSOLE='1')
        with (self.state/'greeter-check.log').open('w+') as log:
            process = subprocess.Popen([self.greeter, '--test-mode', '--theme', str(theme)], stdout=log, stderr=log, env=env)
            try:
                time.sleep(3)
                alive = process.poll() is None
            finally:
                # Popen owns this unreaped child; its PID cannot be reused before wait().
                if process.poll() is None:
                    process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            log.seek(0)
            text = log.read()
        failures = ('is not installed', 'is not a type', 'Cannot load library', 'failed to load', 'Error decoding', 'Cannot open', 'Error loading')
        if not alive or 'WALLPAPER_LAB_GREETER_READY' not in text or any(s.lower() in text.lower() for s in failures):
            raise ValueError('Greeter validation failed; see ' + str(self.state/'greeter-check.log'))

    def run(self):
        action = self.request.get('action', 'status')
        candidate = copy.deepcopy(self.config)
        argument = self.request.get('value', '')
        if action == 'disable':
            candidate['enabled'] = False
            atomic_json(self.config_file, candidate)
            self.config = candidate
        elif action in ('enable', 'check', 'wallpaper', 'preset', 'position', 'crop', 'blur'):
            if action not in ('enable', 'check') and not self.config.get('enabled'):
                raise ValueError('Enable the integration first')
            candidate['enabled'] = True
            if action == 'wallpaper':
                candidate['wallpaper']['source'] = argument
            elif action in ('preset', 'position'):
                candidate['ui'][action] = argument
            elif action in ('crop', 'blur'):
                candidate['wallpaper' if action == 'crop' else 'ui'][action] = argument == 'true'
            previous_config = copy.deepcopy(self.config)
            had_config = self.config_file.exists()
            candidate = self.prepare(candidate)
            try:
                atomic_json(self.config_file, candidate)
                self.config = candidate
                atomic_json(self.state/'verified.json', {'fingerprint': self.fingerprint(candidate), 'tested': time.time()})
            except Exception:
                self.config = previous_config
                shutil.rmtree(self.bundle)
                previous = self.state/'previous'
                if previous.exists():
                    previous.rename(self.bundle)
                if had_config:
                    atomic_json(self.config_file, previous_config)
                else:
                    self.config_file.unlink(missing_ok=True)
                raise
        elif action == 'preview':
            if not self.ready():
                raise ValueError('Validate the integration before previewing')
            # Bounded preview is owned and reaped by this helper; no persistent process or service.
            with (self.state/'greeter-preview.log').open('w') as log:
                process = subprocess.Popen([self.greeter, '--test-mode', '--theme', str(self.bundle)], stdout=log, stderr=log, env=os.environ | {'QT_FORCE_STDERR_LOGGING': '1', 'QT_ASSUME_STDERR_HAS_CONSOLE': '1'})
                try:
                    code = process.wait(timeout=60)
                    if code:
                        raise ValueError("Greeter preview failed; see " + str(self.state/"greeter-preview.log"))
                except subprocess.TimeoutExpired:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
        elif action != 'status':
            raise ValueError('Unknown SDDM action')
        return self.snapshot()

    def snapshot(self):
        preset = self.config.get('ui', {}).get('preset', 'astronaut')
        defaults = ini(self.template/'Themes'/(preset+'.conf')) if preset in self.presets else {}
        blur = value(defaults['General'], 'PartialBlur') == 'true' or value(defaults['General'], 'FullBlur') == 'true' if 'General' in defaults else False
        return dict(ok=True, preset_blur=blur, ready=self.ready(), enabled=self.config.get('enabled', False), config=self.config,
                    backgrounds=self.entries, presets=self.presets, bundle=str(self.bundle), log=str(self.state/'greeter-check.log'))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--request', type=Path, required=True)
    args = parser.parse_args()
    integration = None
    try:
        integration = Integration(json.loads(args.request.read_text()))
        with (integration.state/'lock').open('w') as lock:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            result = integration.run()
    except Exception as error:
        result = integration.snapshot() if integration else {}
        result.update(ok=False, error=str(error))
    print(json.dumps(result))

if __name__ == '__main__':
    main()
