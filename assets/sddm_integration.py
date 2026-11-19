"""Prepare and test an isolated Astronaut greeter. Never change system SDDM configuration."""
import argparse
import configparser
import copy
import fcntl
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import time

IMAGE_DEFAULTS = dict(brightness=0, contrast=1, saturation=1, temperature=0, hue=0, gamma=1, vignette=0, rotation=0, flip_x=0, flip_y=0, zoom=1, x=0, y=0, crop=True)
IMAGE_BOUNDS = dict(brightness=(-1,1), contrast=(0,2), saturation=(0,2), temperature=(-1,1), hue=(-180,180), gamma=(.25,3), vignette=(0,1), rotation=(-180,180), flip_x=(0,1), flip_y=(0,1), zoom=(.25,3), x=(-1,1), y=(-1,1))

def media_kind(path):
    ext = Path(path).suffix.lower()
    return 'video' if ext in {'.avi','.mp4','.mov','.mkv','.m4v','.webm'} else 'gif' if ext == '.gif' else 'image'

def image_settings(config, source=None):
    wallpaper = config.get('wallpaper', {})
    result = IMAGE_DEFAULTS | {'crop': wallpaper.get('crop', True)}
    result.update(wallpaper.get('profiles', {}).get(source or wallpaper.get('source', ''), {}))
    return result

def validated_image(data):
    if not isinstance(data, dict) or set(data) - set(IMAGE_DEFAULTS):
        raise ValueError('Unknown image adjustment')
    result = IMAGE_DEFAULTS.copy()
    for key, number in data.items():
        if key == 'crop':
            if not isinstance(number, bool): raise ValueError('Invalid crop value')
        elif isinstance(number, bool) or not isinstance(number, (float,int)) or not math.isfinite(number) or not IMAGE_BOUNDS[key][0] <= number <= IMAGE_BOUNDS[key][1]:
            raise ValueError('Invalid image adjustment: ' + key)
        result[key] = number
    return result

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
            entries.append(dict(id=str(media.resolve()), label='SDDM · ' + media.stem, title=media.stem.replace('_',' ').title(), origin='sddm', kind=media_kind(media), media=str(media.resolve()), poster='', supported=True))
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
                poster = ''
                if data.get('preview'):
                    try:
                        preview = contained(project.parent, data['preview'])
                        if preview.is_file(): poster = str(preview)
                    except (ValueError, TypeError): pass
                entries.append(dict(title=str(data.get('title', project.parent.name)), origin='local' if origin == 'Local' else 'steam', poster=poster, id=str(project.parent.resolve()), label=origin + ' · ' + str(data.get('title', project.parent.name)) + ((' · ' + kind) if not supported else ''),
                                    media=str(media) if supported else '', supported=supported, kind=media_kind(media) if supported else kind))
            except (ValueError, OSError, TypeError, AttributeError):
                continue
    return entries

class Integration:
    def __init__(self, request):
        self.request = request
        self.config_file = Path(request['config']) / 'sddm.json'
        self.library_file = Path(request['config']) / 'sddm-library.json'
        self.library = json.loads(self.library_file.read_text()) if self.library_file.exists() else {'favorites':[]}
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
        self.assets = Path(request.get('assets', Path(__file__).resolve().parent/'sddm-assets'))
        self.cache = self.state/'previews'
        self.cache.mkdir(exist_ok=True)
        self.greeter = shutil.which('sddm-greeter-qt6')
        self.bundle = self.state / 'prepared'

    def fingerprint(self, config):
        h = hashlib.sha256(json.dumps(config, sort_keys=True).encode())
        for root in (self.template, self.assets, self.bundle):
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

    def resolve_media(self, candidate):
        selected = candidate['wallpaper'].get('source')
        if not selected:
            defaults = ini(self.theme/'Themes/astronaut.conf')
            default = value(defaults['General'], 'Background') if 'General' in defaults else ''
            selected = str((self.theme/default).resolve())
            candidate['wallpaper']['source'] = selected
        entry = next((row for row in self.entries if row['id'] == selected), None)
        if entry and entry['supported'] and Path(entry['media']).is_file():
            return Path(entry['media'])
        # A prepared wallpaper remains usable if its original was removed.
        if selected == self.config.get('wallpaper', {}).get('source'):
            copied = list((self.bundle/'Backgrounds').glob('wallpaper.*'))
            if copied and copied[0].is_file(): return copied[0]
        raise ValueError('Choose an image, GIF or video. Engine scenes and web projects need a future greeter renderer bridge')

    def build_theme(self, candidate, stage):
        if not self.greeter or not self.presets or not self.theme.is_dir():
            raise ValueError('Install SDDM Qt6 and the Astronaut theme first')
        if not (self.assets/'greeter.frag.qsb').is_file():
            raise ValueError('SDDM image controls require a build with Qt6 Shader Tools (qsb)')
        source = self.resolve_media(candidate)
        probe = subprocess.run(['ffprobe', '-v', 'error', '-protocol_whitelist', 'file', '-select_streams', 'v:0',
                                '-show_entries', 'stream=width,height', '-of', 'json', str(source)], capture_output=True, text=True, timeout=20)
        streams = json.loads(probe.stdout).get('streams', [])
        if probe.returncode or not streams or streams[0].get('width', 0) < 1:
            raise ValueError('Cannot decode selected media')
        preset = candidate['ui']['preset']
        if preset not in self.presets: raise ValueError('Unknown login UI preset')
        for name in ('Components', 'Assets'):
            shutil.copytree(self.template/name, stage/name)
        if (self.theme/'Fonts').is_dir(): shutil.copytree(self.theme/'Fonts', stage/'Fonts')
        for name in ('LICENSE', 'metadata.desktop'): shutil.copy2(self.template/name, stage/name)
        for name in ('LabBackground.qml', 'greeter.frag.qsb'): shutil.copy2(self.assets/name, stage/name)
        text = (self.template/'Main.qml').read_text()
        begin = text.index('        AnimatedImage {\n            id: backgroundImage')
        end = text.index('        MouseArea {', begin)
        text = text[:begin] + """        LabBackground {
            id: backgroundImage
            source: Qt.resolvedUrl(config.Background)
            height: parent.height
            width: config.HaveFormBackground == "true" && config.FormPosition != "center" && config.PartialBlur != "true" ? parent.width - formBackground.width : parent.width
            anchors.left: leftleft || leftcenter ? formBackground.right : undefined
            anchors.right: rightright || rightcenter ? formBackground.left : undefined
        }

""" + text[end:]
        (stage/'ThemeMain.qml').write_text(text)
        (stage/'Main.qml').write_text('import QtQuick\nimport "."\nThemeMain { Component.onCompleted: console.log("WALLPAPER_LAB_GREETER_READY") }\n')
        metadata = ini(stage/'metadata.desktop')
        metadata['SddmGreeterTheme']['ConfigFile'] = 'theme.conf'
        with (stage/'metadata.desktop').open('w') as out: metadata.write(out)
        background = stage/'Backgrounds'/('wallpaper' + source.suffix.lower())
        background.parent.mkdir()
        shutil.copyfile(source, background)
        config = ini(self.template/'Themes'/(preset+'.conf'))
        general = config['General']
        general.update(Background='Backgrounds/'+background.name, background='Backgrounds/'+background.name,
                       BackgroundPlaceholder='', ScreenWidth='', ScreenHeight='', PauseBackground='false', BackgroundSpeed='1.0')
        image = validated_image(image_settings(candidate))
        general['CropBackground'] = 'true' if image['crop'] else 'false'
        general['LabWidth'], general['LabHeight'] = str(streams[0]['width']), str(streams[0]['height'])
        for key, number in image.items():
            general['Lab'+''.join(part.title() for part in key.split('_'))] = str(number)
        position = candidate['ui'].get('position', 'preset')
        if position != 'preset':
            if position not in ('left', 'center', 'right'): raise ValueError('Invalid login form position')
            general['FormPosition'] = position
        if 'blur' in candidate['ui']:
            general['PartialBlur'] = 'true' if candidate['ui']['blur'] else 'false'
            general['FullBlur'] = 'false'
        with (stage/'theme.conf').open('w') as out: config.write(out)
        stage.chmod(0o755)
        for path in stage.rglob('*'): path.chmod(0o755 if path.is_dir() else 0o644)

    def prepare(self, candidate):
        stage = Path(tempfile.mkdtemp(prefix='stage-', dir=self.state))
        try:
            self.build_theme(candidate, stage)
            self.check(stage)
            previous = self.state/'previous'
            if previous.exists(): shutil.rmtree(previous)
            if self.bundle.exists(): self.bundle.rename(previous)
            try: stage.rename(self.bundle)
            except OSError:
                if previous.exists(): previous.rename(self.bundle)
                raise
            return candidate
        finally:
            if stage.exists(): shutil.rmtree(stage)

    def preview_config(self, kind, identifier):
        if kind == 2: return copy.deepcopy(self.config)
        if identifier not in self.presets: raise ValueError('Unknown login UI preset')
        defaults = ini(self.template/'Themes'/(identifier+'.conf'))
        source = str((self.theme/value(defaults['General'], 'Background')).resolve())
        if not any(row['id'] == source and row['supported'] for row in self.entries):
            source = next((row['id'] for row in self.entries if row['origin']=='sddm' and row['supported']), '')
        return dict(enabled=True, wallpaper={'source':source}, ui={'preset':identifier})

    def thumbnail_path(self, kind, identifier):
        h = hashlib.sha256(('sddm-library-v1:'+str(kind)+identifier).encode())
        paths = []
        if kind == 0:
            entry = next((row for row in self.entries if row['id']==identifier), None)
            if not entry: return ''
            paths = [Path(entry['media'] or entry['poster'])] if entry['media'] or entry['poster'] else []
            if not paths: return ''
        else:
            config = self.preview_config(kind, identifier)
            h.update(json.dumps(config, sort_keys=True).encode())
            try: paths = [self.resolve_media(config)]
            except ValueError: return ''
            for root in (self.template, self.assets): paths += [p for p in root.rglob('*') if p.is_file()]
        for path in paths:
            try:
                stat = path.stat(); h.update(f'{path}:{stat.st_size}:{stat.st_mtime_ns}'.encode())
            except OSError: return ''
        return str(self.cache/(h.hexdigest()+'.png'))

    @staticmethod
    def stop_child(process):
        # An unreaped Popen child cannot have its PID reused.
        if process.poll() is None: process.terminate()
        try: process.wait(timeout=3)
        except subprocess.TimeoutExpired: process.kill(); process.wait()

    def capture(self, stage, destination):
        temporary = stage/'capture.png'
        # The greeter sets its root size. Capture a fixed-size child, not that root.
        (stage/'Main.qml').write_text('import QtQuick\nimport "."\nItem { ThemeMain { id: sample; width:1280; height:720; Component.onCompleted: console.log("WALLPAPER_LAB_GREETER_READY") } Timer { interval:1600; running:true; repeat:false; onTriggered: sample.grabToImage(function(result) { if (result.saveToFile('+json.dumps(str(temporary))+')) console.log("WALLPAPER_LAB_CAPTURE_READY"); }) } }\n')
        env = os.environ | dict(QT_QPA_PLATFORM='offscreen', QT_QUICK_BACKEND='rhi', QSG_RHI_BACKEND='opengl', LIBGL_ALWAYS_SOFTWARE='1', QT_FORCE_STDERR_LOGGING='1', QT_ASSUME_STDERR_HAS_CONSOLE='1')
        with destination.with_suffix('.log').open('w+') as log:
            process = subprocess.Popen([self.greeter,'--test-mode','--theme',str(stage)],stdout=log,stderr=log,env=env)
            try:
                deadline = time.monotonic()+15
                while time.monotonic()<deadline and process.poll() is None:
                    time.sleep(.1);log.seek(0);text=log.read()
                    if 'WALLPAPER_LAB_CAPTURE_READY' in text:
                        if 'WALLPAPER_LAB_SHADER_ERROR' in text: raise ValueError('SDDM preview shader failed')
                        temporary.replace(destination);return
                raise ValueError('SDDM preview capture failed; see '+str(destination.with_suffix('.log')))
            finally: self.stop_child(process)

    def thumbnail(self):
        request = json.loads(self.request.get('value','{}'))
        kind, identifier = request['kind'], request.get('id','')
        if kind not in (0,1,2): raise ValueError('Invalid preview kind')
        target = self.thumbnail_path(kind,identifier)
        if not target: raise ValueError('No preview media available')
        destination=Path(target)
        with destination.with_suffix('.lock').open('w') as lock:
            fcntl.flock(lock,fcntl.LOCK_EX)
            if not destination.is_file():
                with tempfile.TemporaryDirectory(prefix='capture-',dir=self.state) as temp:
                    stage=Path(temp)
                    if kind==0:
                        entry=next(row for row in self.entries if row['id']==identifier)
                        source=entry['media'] or entry['poster'];temporary=stage/'poster.png'
                        subprocess.run(['ffmpeg','-v','error','-nostdin','-protocol_whitelist','file','-i',source,'-frames:v','1','-vf','scale=640:640:force_original_aspect_ratio=decrease','-threads','1','-y',str(temporary)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=20)
                        temporary.replace(destination)
                    else:
                        self.build_theme(self.preview_config(kind,identifier),stage)
                        self.capture(stage,destination)
        return dict(ok=True,thumbnail=target)

    def check(self, theme):
        env = os.environ.copy()
        env.update(QT_QPA_PLATFORM='offscreen', QT_QUICK_BACKEND='rhi', QSG_RHI_BACKEND='opengl', LIBGL_ALWAYS_SOFTWARE='1', QT_FORCE_STDERR_LOGGING='1', QT_ASSUME_STDERR_HAS_CONSOLE='1')
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
        failures = ('is not installed', 'is not a type', 'Cannot load library', 'failed to load', 'Error decoding', 'Cannot open', 'Error loading', 'WALLPAPER_LAB_SHADER_ERROR')
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
        elif action == 'favorite':
            if not any(row['id']==argument for row in self.entries): raise ValueError('Unknown wallpaper')
            favorites = self.library.setdefault('favorites',[])
            if argument in favorites: favorites.remove(argument)
            else: favorites.append(argument)
            atomic_json(self.library_file,self.library)
        elif action in ('enable', 'check', 'wallpaper', 'preset', 'position', 'crop', 'blur', 'edit'):
            if action not in ('enable', 'check') and not self.config.get('enabled'):
                raise ValueError('Enable the integration first')
            candidate['enabled'] = True
            if action == 'wallpaper':
                candidate['wallpaper']['source'] = argument
            elif action in ('preset', 'position'):
                candidate['ui'][action] = argument
            elif action == 'blur':
                candidate['ui']['blur'] = argument == 'true'
            elif action in ('crop','edit'):
                source = candidate['wallpaper']['source']
                if action == 'edit':
                    edit = json.loads(argument)
                    if edit['source'] != source: raise ValueError('Selected wallpaper changed before editing')
                    image = validated_image(edit['values'])
                else: image = image_settings(candidate) | {'crop':argument == 'true'}
                candidate['wallpaper'].setdefault('profiles',{})[source] = image
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
        elif action in ('preview', 'preview-wallpaper'):
            if not self.ready():
                raise ValueError('Validate the integration before previewing')
            preview_theme = self.bundle
            temporary = None
            if action == 'preview-wallpaper':
                temporary = tempfile.TemporaryDirectory(prefix='wallpaper-preview-',dir=self.state)
                preview_theme = Path(temporary.name)
                self.build_theme(copy.deepcopy(self.config), preview_theme)
                (preview_theme/'Main.qml').write_text('import QtQuick\nimport "."\nLabBackground { source: Qt.resolvedUrl(config.Background); Component.onCompleted: console.log("WALLPAPER_LAB_GREETER_READY") }\n')
            # Bounded preview is owned and reaped by this helper; no persistent process or service.
            with (self.state/'greeter-preview.log').open('w') as log:
                process = subprocess.Popen([self.greeter, '--test-mode', '--theme', str(preview_theme)], stdout=log, stderr=log, env=os.environ | {'QT_FORCE_STDERR_LOGGING': '1', 'QT_ASSUME_STDERR_HAS_CONSOLE': '1'})
                try:
                    code = process.wait(timeout=60)
                    # SDDM's own SIGTERM handler exits with 255 on some builds.
                    # Accept that only after a loaded preview reports the signal.
                    output = (self.state/'greeter-preview.log').read_text()
                    closed = code == 255 and 'WALLPAPER_LAB_GREETER_READY' in output and 'Signal received: SIGTERM' in output
                    if code not in (0, -signal.SIGTERM, 128+signal.SIGTERM) and not closed:
                        raise ValueError("Greeter preview failed; see " + str(self.state/"greeter-preview.log"))
                except subprocess.TimeoutExpired:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                finally:
                    self.stop_child(process)
                    if temporary: temporary.cleanup()
        elif action != 'status':
            raise ValueError('Unknown SDDM action')
        return self.snapshot()

    def snapshot(self):
        preset = self.config.get('ui', {}).get('preset', 'astronaut')
        defaults = ini(self.template/'Themes'/(preset+'.conf')) if preset in self.presets else {}
        blur = value(defaults['General'], 'PartialBlur') == 'true' or value(defaults['General'], 'FullBlur') == 'true' if 'General' in defaults else False
        backgrounds = [row | {'thumbnail':self.thumbnail_path(0,row['id']), 'favorite':row['id'] in self.library.get('favorites',[])} for row in self.entries]
        previews = [dict(id=p,title=p.replace('_',' ').replace('-',' ').title(),thumbnail=self.thumbnail_path(1,p)) for p in self.presets]
        final = self.thumbnail_path(2,'') if self.config.get('enabled') else ''
        return dict(ok=True, final_preview=final, ui_previews=previews, image=image_settings(self.config), preset_blur=blur, ready=self.ready(), enabled=self.config.get('enabled', False), config=self.config,
                    backgrounds=backgrounds, presets=self.presets, bundle=str(self.bundle), log=str(self.state/'greeter-check.log'))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--request', type=Path, required=True)
    args = parser.parse_args()
    integration = None
    try:
        integration = Integration(json.loads(args.request.read_text()))
        if integration.request.get('action') == 'thumbnail':
            result = integration.thumbnail()
        else:
            with (integration.state/'lock').open('w') as lock:
                fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
                # Read configuration under the mutation lock, after any prior writer.
                if integration.config_file.exists(): integration.config=json.loads(integration.config_file.read_text())
                result = integration.run()
    except Exception as error:
        result = integration.snapshot() if integration else {}
        result.update(ok=False, error=str(error))
    print(json.dumps(result))

if __name__ == '__main__':
    main()
