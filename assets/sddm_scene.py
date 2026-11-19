"""Bounded native scene feeds for SDDM previews; no desktop renderer ownership."""
from contextlib import contextmanager
import configparser
import json
import math
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import threading
import time


def screens_for(request):
    result = []
    for row in request.get('screens', [])[:16]:
        try:
            numbers = [float(row[k]) for k in ('x', 'y', 'width', 'height')]
            x, y, width, height = numbers
            if not all(math.isfinite(n) and abs(n) <= 32768 for n in numbers) or min(width, height) < 1:
                continue
            rect = dict(x=x, y=y, width=width, height=height)
            if not any(all(old[k] == v for k, v in rect.items()) for old in result):
                result.append(rect | {'name': str(row.get('name', 'Display'))})
        except (ValueError, KeyError, TypeError):
            continue
    return result or [dict(name='Display', x=0, y=0, width=1280, height=720)]


def desktop_rect(screens):
    x, y = min(s['x'] for s in screens), min(s['y'] for s in screens)
    return dict(x=x, y=y, width=max(s['x']+s['width'] for s in screens)-x,
                height=max(s['y']+s['height'] for s in screens)-y)


def scene_size(request, config):
    screens = screens_for(request)
    rect = desktop_rect(screens) if config.get('wallpaper', {}).get('display_mode') == 'span' else max(screens, key=lambda s: s['width']*s['height'])
    scale = min(1, 2560/rect['width'], 1440/rect['height'])
    return max(2, round(rect['width']*scale)), max(2, round(rect['height']*scale))


def identity(process):
    proc = Path('/proc', str(process.pid))
    return (proc.joinpath('stat').read_text().rsplit(')', 1)[1].split()[19], proc.joinpath('exe').resolve())


def stop_renderer(process, expected):
    if process.poll() is not None:
        return
    for sig, timeout in ((signal.SIGTERM, 5), (signal.SIGKILL, 3)):
        if identity(process) != expected or b'--render' not in Path('/proc', str(process.pid), 'cmdline').read_bytes().split(b'\0'):
            raise ValueError('Scene renderer identity changed; refusing to signal it')
        fd = os.pidfd_open(process.pid)
        try:
            if identity(process) != expected:
                raise ValueError('Scene renderer identity changed')
            signal.pidfd_send_signal(fd, sig)
        finally:
            os.close(fd)
        try:
            process.wait(timeout=timeout)
            return
        except subprocess.TimeoutExpired:
            pass
    raise ValueError('Scene renderer did not stop')


def read_frame(path, width, height):
    data = path.read_bytes()
    header = f'P6\n{width} {height}\n255\n'.encode()
    if not data.startswith(header) or len(data) != len(header)+width*height*3:
        raise ValueError('Incomplete scene frame')
    return data


@contextmanager
def scene_runtime(stage, log_path):
    """Run one shared hidden renderer for all greeter windows, then reap it."""
    metadata = stage/'scene.json'
    if not metadata.exists():
        yield None
        return
    scene = json.loads(metadata.read_text())
    program, project, assets = (Path(scene[k]).resolve() for k in ('renderer', 'project', 'assets'))
    if not program.is_file() or not project.joinpath('project.json').is_file() or not assets.is_dir():
        raise ValueError('Scene renderer, project or Wallpaper Engine assets unavailable')
    if json.loads(project.joinpath('project.json').read_text()).get('type', '').lower() != 'scene':
        raise ValueError('The native SDDM bridge accepts scene projects only')
    width, height = int(scene['width']), int(scene['height'])
    if not 1 < width <= 2560 or not 1 < height <= 1440:
        raise ValueError('Invalid scene feed size')
    original = (stage/'theme.conf').read_bytes()
    process = None
    expected = None
    heartbeat = None
    done = threading.Event()
    with tempfile.TemporaryDirectory(prefix='wallpaper-lab-sddm-', dir=os.getenv('XDG_RUNTIME_DIR')) as temporary, log_path.open('w') as log:
        feed = Path(temporary)
        request = dict(path=str(project), output='default', ppm=True, fps=30)
        def renew():
            while not done.is_set():
                temp = feed/'request.tmp'
                temp.write_text(json.dumps(request))
                temp.replace(feed/'request.json')
                done.wait(.5)
        try:
            heartbeat = threading.Thread(target=renew, daemon=True)
            heartbeat.start()
            args = [str(program), '--render', '--ready-file', str(feed/'ready'), '--lab-hidden', '--lab-parent', str(os.getpid()),
                    '--assets-dir', str(assets), '--fps', '30', '--volume', '0', '--no-audio-processing', '--disable-mouse',
                    '--disable-parallax', '--no-fullscreen-pause', '--lab-project', str(project), '--lab-feed-dir', str(feed),
                    '--scaling', 'fit', '--clamp', 'border', '--window', f'0x0x{width}x{height}', str(project)]
            process = subprocess.Popen(args, stdout=log, stderr=log)
            expected = identity(process)
            if expected[1] != program:
                raise ValueError('Unexpected scene renderer executable')
            deadline = time.monotonic()+25
            while time.monotonic() < deadline and process.poll() is None:
                try:
                    frame = read_frame(feed/'scene.ppm', width, height)
                    if (feed/'ready').is_file():
                        break
                except (OSError, ValueError):
                    pass
                time.sleep(.1)
            else:
                raise ValueError('Scene did not produce a valid frame; see '+str(log_path))
            # A genuine rendered fallback remains available if the live feed stops.
            (stage/'Backgrounds/wallpaper.ppm').write_bytes(frame)
            config = configparser.ConfigParser(interpolation=None)
            config.optionxform = str
            config.read(stage/'theme.conf')
            config['General']['LabSceneFrame'] = (feed/'scene.ppm').as_uri()
            with (stage/'theme.conf').open('w') as target:
                config.write(target)
            yield dict(pid=process.pid, start=expected[0], frame=feed/'scene.ppm', width=width, height=height)
            if process.poll() is not None:
                raise ValueError('Scene renderer exited during preview; see '+str(log_path))
        finally:
            done.set()
            if heartbeat:
                heartbeat.join(timeout=2)
            try:
                if process is not None and expected is not None:
                    stop_renderer(process, expected)
            finally:
                (stage/'theme.conf').write_bytes(original)
