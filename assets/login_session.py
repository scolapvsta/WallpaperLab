"""Dedicated greetd compositor session with bounded startup and native fallback."""
from pathlib import Path
import json,os,signal,subprocess,sys,tempfile,time


def compositor_config(bundle,inside_command):
    d=json.loads((bundle/'login.json').read_text())
    lines=['hl.monitor({output="",mode="preferred",position="auto",scale=1})',
           'hl.config({ animations={enabled=false}, misc={disable_hyprland_logo=true,disable_splash_rendering=true}, ecosystem={enforce_permissions=true} })',
           'hl.permission(".*", "screencopy", "deny")','hl.permission(".*", "plugin", "deny")']
    for screen in d.get('monitors',[]):
        name=screen.get('name','')
        if not name or len(name)>128:continue
        width,height=int(screen['width']),int(screen['height']);x,y=int(screen['x']),int(screen['y']);scale=float(screen.get('scale',1));transform=int(screen.get('transform',0))
        if not(0<width<=32768 and 0<height<=32768 and .25<=scale<=8 and 0<=transform<=7):raise ValueError('Invalid saved monitor geometry')
        lines.append('hl.monitor({output=%s,mode=%s,position=%s,scale=%s,transform=%d})'%(json.dumps(name),json.dumps(f'{width}x{height}'),json.dumps(f'{x}x{y}'),scale,transform))
    lines.append('hl.on("hyprland.start",function() hl.exec_cmd('+json.dumps(inside_command)+') end)')
    return '\n'.join(lines)+'\n'


def process_identity(pid):
    proc=Path('/proc',str(pid))
    return ((proc/'stat').read_text().rsplit(')',1)[1].split()[19],str((proc/'exe').resolve()))

def stop_child(child,identity,group=False):
    if child.poll()is not None:return
    def owned():
        try:return process_identity(child.pid)==identity
        except OSError:return False
    try:fd=os.pidfd_open(child.pid)
    except ProcessLookupError:return
    try:
        if not owned():raise ValueError('Login renderer identity changed')
        def send(sig):
            if group and os.getpgid(child.pid)==child.pid:os.killpg(child.pid,sig)
            else:signal.pidfd_send_signal(fd,sig)
        send(signal.SIGTERM)
        try:child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            if owned():send(signal.SIGKILL)
            child.wait(timeout=5)
    finally:os.close(fd)


def inside(bundle):
    runtime=bundle/'runtime';env=os.environ.copy();env['LD_LIBRARY_PATH']=':'.join(str(runtime/p)for p in ['.','lib','deps'])
    state=Path(os.environ.get('WALLPAPER_LAB_LOGIN_STATE','/var/lib/wallpaper-lab-login'));env['WALLPAPER_LAB_LOGIN_STATE']=str(state)
    cache=Path(os.environ['XDG_RUNTIME_DIR'])/'wallpaper-lab-login';cache.mkdir(mode=0o700,exist_ok=True)
    success=False
    for name in ['login.json','fallback.json']:
        ready=cache/'ready';ready.unlink(missing_ok=True)
        with(cache/'renderer.log').open('w')as log:
            child=subprocess.Popen([str(runtime/'wallpaper-lab'),'--login-greeter',str(bundle/name),'--outputs',str(ready)],env=env,stdout=log,stderr=log)
            try:identity=process_identity(child.pid)
            except OSError:
                child.wait();continue
            try:
                deadline=time.monotonic()+30
                while child.poll()is None and not ready.exists()and time.monotonic()<deadline:time.sleep(.1)
                if not ready.exists():continue
                # Drawing failures and crashes fall back, not a silent background-only greeter.
                child.wait()
                if child.returncode==0 and 'WALLPAPER_LAB_LOGIN_STARTED' in (cache/'renderer.log').read_text():success=True;break
            finally:stop_child(child,identity)
    marker=cache/('session-started'if success else'failed');marker.touch(mode=0o600)
    # End only this private compositor. greetd starts the authenticated session afterwards.
    subprocess.run(['hyprctl','eval','hl.dispatch(hl.dsp.exit())'],check=False,stdout=subprocess.DEVNULL)
    return 0 if success else 1


def main():
    if len(sys.argv)==3 and sys.argv[1]=='--inside':return inside(Path(sys.argv[2]).resolve())
    if len(sys.argv)!=2 or not os.getenv('GREETD_SOCK'):raise ValueError('This session must be launched by greetd')
    bundle=Path(sys.argv[1]).resolve();state=Path('/var/lib/wallpaper-lab-login')
    with tempfile.TemporaryDirectory(prefix='wallpaper-lab-greeter-',dir=os.environ['XDG_RUNTIME_DIR'])as directory:
        work=Path(directory);config=work/'hyprland.lua'
        # Installed bundle paths contain only a fixed prefix and a numeric release identifier.
        command='/usr/bin/python3 '+str(bundle/'runtime/login_session.py')+' --inside '+str(bundle)
        config.write_text(compositor_config(bundle,command))
        env=os.environ|{'XDG_CONFIG_HOME':str(work/'config'),'XDG_CACHE_HOME':str(state/'cache'),'XDG_STATE_HOME':str(state/'state'),'WALLPAPER_LAB_LOGIN_STATE':str(state)}
        for key in ['WAYLAND_DISPLAY','DISPLAY','HYPRLAND_INSTANCE_SIGNATURE','DBUS_SESSION_BUS_ADDRESS','LD_PRELOAD','LD_LIBRARY_PATH']:env.pop(key,None)
        cache=Path(os.environ['XDG_RUNTIME_DIR'])/'wallpaper-lab-login';cache.mkdir(mode=0o700,exist_ok=True)
        for name in ['session-started','failed','ready']:(cache/name).unlink(missing_ok=True)
        with(cache/'compositor.log').open('w')as log:
            child=subprocess.Popen(['dbus-run-session','--','Hyprland','--config',str(config)],env=env,stdout=log,stderr=log,start_new_session=True)
            try:identity=process_identity(child.pid)
            except OSError:identity=None
            if identity:
                try:
                    deadline=time.monotonic()+75
                    while child.poll()is None and not(cache/'ready').exists()and time.monotonic()<deadline:time.sleep(.1)
                    if(cache/'ready').exists():child.wait()
                finally:stop_child(child,identity,group=True)
        marker=cache/'session-started'
        if marker.exists():marker.unlink();return 0
    # If graphics cannot start, keep login possible on the allocated VT.
    os.execvp('agreety',['agreety','--cmd','/usr/bin/start-hyprland'])

if __name__=='__main__':sys.exit(main())
