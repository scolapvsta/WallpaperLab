"""Explicit Hyprland graphical check: local GIF animation and mock greetd UI.

Uses isolated XDG state and a fake IPC socket. It never authenticates a system
user or changes the display manager. Requires Pillow, grim and ydotool.
"""
from pathlib import Path
import argparse,importlib.util,json,os,socket,struct,subprocess,threading,time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--preset',type=Path);a=p.parse_args()
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('session',root/'assets/login_session.py');session=importlib.util.module_from_spec(spec);spec.loader.exec_module(session)
input_socket=Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-login-input-{os.getpid()}.sock';auth_socket=Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-login-auth-{os.getpid()}.sock'
env=os.environ|dict(XDG_CONFIG_HOME=str(out/'config'),XDG_STATE_HOME=str(out/'state'),XDG_CACHE_HOME=str(out/'cache'),XDG_DATA_HOME=str(out/'data'),WALLPAPER_LAB_LIBRARY=str(out/'empty-library'),YDOTOOL_SOCKET=str(input_socket),WALLPAPER_LAB_LOGIN_STATE=str(out/'identity'))
(out/'empty-library').mkdir(exist_ok=True)
def run(*args):return subprocess.check_output(list(map(str,args)),env=env,text=True)
def wait(fn,timeout=20):
    end=time.monotonic()+timeout
    while time.monotonic()<end:
        try:
            if value:=fn():return value
        except(OSError,KeyError,ValueError,StopIteration):pass
        time.sleep(.05)
    raise AssertionError('Timed out')
def window(child):return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==child.pid)
def focus(child):
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{child.pid}"}}))');time.sleep(.1)
    assert json.loads(run('hyprctl','activewindow','-j'))['pid']==child.pid

def click(child,x,y):
    assert session.process_identity(child.pid)==identity
    assert b'--login-preview' in (Path('/proc')/str(child.pid)/'cmdline').read_bytes().split(b'\0')
    focus(child);w=window(child);px,py=round(w['at'][0]+x*w['size'][0]),round(w['at'][1]+y*w['size'][1])
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={px},y={py},relative=false}}))');time.sleep(.15)
    assert json.loads(run('hyprctl','activewindow','-j'))['pid']==child.pid
    run('ydotool','click','0x40');time.sleep(.1);run('ydotool','click','0x80');time.sleep(.25)

def key(child,*keys):focus(child);run('ydotool','key',*keys)
def text(child,value):focus(child);run('ydotool','type','--key-delay','8',value)
def shot(child,path):
    w=window(child);x,y=w['at'];width,height=w['size'];run('grim','-g',f'{x},{y} {width}x{height}',path)
frames=[Image.new('RGB',(640,360),c)for c in ['#2060d0','#e08020']];media=out/'animation.gif';frames[0].save(media,save_all=True,append_images=frames[1:],duration=500,loop=0)
run(app,'--library-path','local',out/'empty-library');source=json.loads(run(app,'--import',media))[0];run(app,'--login','set',json.dumps({'enabled':True,'source':source,'mode':'mirror','language':'en'}));
if a.preset:run(app,'--login','set',json.dumps(json.loads(a.preset.read_text())['data']))
prepared=Path(json.loads(run(app,'--login','prepare'))['prepared'])
results={};child=None;identity=None;daemon=None;server=None;thread=None;errors=[]
try:
    with(out/'input.log').open('w')as log:daemon=subprocess.Popen(['ydotoold','--socket-path='+str(input_socket)],stdout=log,stderr=log)
    wait(input_socket.exists)
    ready=out/'gif-ready';ready.unlink(missing_ok=True)
    with(out/'gif.log').open('w')as log:child=subprocess.Popen([str(app),'--login-preview',str(prepared),'--window',str(ready)],env=env,stdout=log,stderr=log)
    identity=session.process_identity(child.pid);wait(ready.exists);wait(lambda:window(child));focus(child);colors=[]
    for n in range(9):
        path=out/f'gif-{n}.png';shot(child,path);im=Image.open(path);colors.append(im.getpixel((im.width//10,im.height//2))[:3]);time.sleep(.12)
    assert any(b>r+50 for r,g,b in colors)and any(r>b+50 for r,g,b in colors),colors
    if a.preset:
        form=json.loads(prepared.read_text())['widgets']['login'];layout=form['astronaut']
        if layout['system_buttons']:
            click(child,form['x']+form['width']*(.5+layout['action_centers'][0]),form['y']+form['height']*(layout['actions_y']+.02));shot(child,out/'system-confirmation.png')
            click(child,.56,.56)  # Cancel; no system action is ever executed by this check.
            results['preview_system_confirmation_and_cancel']=True
        if layout['virtual_keyboard']:
            click(child,form['x']+form['width']/2,form['y']+form['height']*(layout['keyboard_y']+.015));shot(child,out/'virtual-keyboard.png')
            click(child,form['x']+form['width']*.05,form['y']+form['height']*.745);key(child,'1:1','1:0');key(child,'14:1','14:0')
            results['native_virtual_keyboard_opens_types_and_closes']=True
    text(child,'preview-test');shot(child,out/'preview-password.png');key(child,'15:1','15:0');key(child,'28:1','28:0');shot(child,out/'session-dropdown.png');key(child,'108:1','108:0');key(child,'28:1','28:0');key(child,'15:1','15:0');key(child,'15:1','15:0');key(child,'28:1','28:0');key(child,'1:1','1:0');child.wait(timeout=5);assert child.returncode==0
    results['native_gif_changes_frames']=True;assert 'preview-test'not in (out/'gif.log').read_text();assert not(out/'identity/last-session.json').exists();results['interactive_preview_without_authentication_or_persistence']=True;results['session_dropdown_keyboard_and_escape']=True
    # The actual greeter entry point is exercised against a test-only server.
    server=socket.socket(socket.AF_UNIX,socket.SOCK_STREAM);server.bind(str(auth_socket));server.listen(1);server.settimeout(20);requests=[]
    def ipc():
        try:
            conn,_=server.accept();conn.settimeout(20)
            with conn:
                def exact(n):
                    data=b''
                    while len(data)<n:
                        part=conn.recv(n-len(data));assert part;data+=part
                    return data
                for expected,response in [('create_session',{'type':'auth_message','auth_message_type':'secret','auth_message':'Test fixture password:'}),('post_auth_message_response',{'type':'success'}),('start_session',{'type':'success'})]:
                    length=struct.unpack('=I',exact(4))[0];request=json.loads(exact(length));assert request['type']==expected;requests.append(expected)
                    if expected=='create_session':assert request['username']=='demo'
                    if expected=='post_auth_message_response':assert request['response']=='test-only'
                    if expected=='start_session':assert request['cmd']and any(e=='XDG_SESSION_TYPE=wayland'for e in request['env'])
                    data=json.dumps(response).encode();conn.sendall(struct.pack('=I',len(data))+data)
        except BaseException as e:errors.append(repr(e))
    thread=threading.Thread(target=ipc);thread.start();ready=out/'greeter-ready';ready.unlink(missing_ok=True)
    doc=json.loads(prepared.read_text());doc['username']='';prepared.write_text(json.dumps(doc));(out/'identity/last-session.json').unlink(missing_ok=True)
    with(out/'greeter.log').open('w')as log:child=subprocess.Popen([str(app),'--login-greeter',str(prepared),'--window',str(ready)],env=env|{'GREETD_SOCK':str(auth_socket)},stdout=log,stderr=log)
    identity=session.process_identity(child.pid);wait(ready.exists);wait(lambda:window(child));text(child,'demo');key(child,'28:1','28:0');wait(lambda:requests==['create_session']);time.sleep(.3);text(child,'testonly');key(child,'102:1','102:0');key(child,*(['106:1','106:0']*4));text(child,'-');key(child,'107:1','107:0');text(child,'X');key(child,'14:1','14:0');shot(child,out/'secret-masked.png');key(child,'28:1','28:0');child.wait(timeout=10);thread.join(timeout=5);assert not thread.is_alive()and not errors,errors
    assert child.returncode==0 and requests[-1]=='start_session';saved=json.loads((out/'identity/last-session.json').read_text());assert set(saved)=={'username','session'}and saved['username']=='demo'
    log=(out/'greeter.log').read_text();assert 'test-only'not in log and 'WALLPAPER_LAB_LOGIN_STARTED'in log
    results['native_greeter_keyboard_and_greetd_flow']=True;results['only_identity_persisted']=True;results['password_cursor_home_end_insert_backspace']=True
    (out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results))
finally:
    if child and identity:session.stop_child(child,identity)
    if daemon and daemon.poll()is None:daemon.terminate();daemon.wait()
    if server:server.close()
    if thread:thread.join(timeout=21)
    input_socket.unlink(missing_ok=True);auth_socket.unlink(missing_ok=True)
