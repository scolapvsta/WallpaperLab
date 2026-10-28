#!/usr/bin/env python3
"""Run under dbus-run-session; no changes to the user's actual media players."""
import pathlib,subprocess,time,os,json,sys
out=pathlib.Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True);app=pathlib.Path(sys.argv[1]).resolve();env=os.environ|{'WALLPAPER_LAB_TEXT_TRACE':str(out/'text.log'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])}
if '--service'in sys.argv:
 import dbus,dbus.service,dbus.mainloop.glib
 from gi.repository import GLib
 dbus.mainloop.glib.DBusGMainLoop(set_as_default=True);bus=dbus.SessionBus();name=dbus.service.BusName('org.mpris.MediaPlayer2.WallpaperLabTest',bus)
 class Player(dbus.service.Object):
  @dbus.service.method('org.freedesktop.DBus.Properties',in_signature='ss',out_signature='v')
  def Get(self,interface,key):
   return {'Metadata':dbus.Dictionary({'xesam:title':dbus.String('日本語 한국어 हिन्दी العربية'),'xesam:artist':dbus.Array(['Artist'],signature='s'),'mpris:length':dbus.Int64(180000000)},signature='sv'),'PlaybackStatus':dbus.String('Playing'),'Position':dbus.Int64(30000000)}[key]
 player=Player(bus,'/org/mpris/MediaPlayer2');(out/'service-ready').touch();GLib.MainLoop().run();sys.exit()
service=subprocess.Popen([sys.executable,__file__,str(app),str(out),'--service'],env=env);child=None
try:
 for _ in range(60):
  if(out/'service-ready').exists():break
  time.sleep(.05)
 fixture=out/'project';fixture.mkdir(exist_ok=True);script="""export function init(v){if(MediaPlaybackEvent.PLAYBACK_PLAYING!==1)throw Error('enum');return v;}export function mediaPropertiesChanged(e){console.error('MEDIA_TITLE:'+e.title);}export function mediaTimelineChanged(e){console.error('MEDIA_TIME:'+e.position+':'+e.duration);}export function update(v){return v;}""";scene={'camera':{'center':'0 0 0','eye':'0 0 1','up':'0 1 0'},'general':{'orthogonalprojection':{'width':640,'height':360},'clearcolor':'0 0 0'},'objects':[{'id':1,'name':'metadata','image':'models/util/solidlayer.json','origin':'320 180 0','size':'640 360','visible':{'value':True,'script':script}}]};(fixture/'scene.json').write_text(json.dumps(scene));(fixture/'project.json').write_text(json.dumps({'title':'Metadata test','type':'scene','file':'scene.json'}));
 if '--reze'in sys.argv:fixture=pathlib.Path.home()/'.local/share/Steam/steamapps/workshop/content/431960/3572814850'
 assets=pathlib.Path.home()/'.local/share/Steam/steamapps/common/wallpaper_engine/assets';log=open(out/'renderer.log','w');child=subprocess.Popen([str(app),'--render','--ready-file',str(out/'ready'),'--assets-dir',str(assets),'--screen-root','DP-6','--no-fullscreen-pause','--volume','0','--fps','30',str(fixture)],env=env,stdout=log,stderr=log,start_new_session=True)
 for _ in range(100):
  time.sleep(.1);text=(out/'renderer.log').read_text()
  if '--reze'in sys.argv:
   text=(out/'text.log').read_text()if(out/'text.log').exists()else''
   if '日本語 한국어 हिन्दी العربية'in text and 'Artist'in text:break
  elif'MEDIA_TIME:30:180'in text:break
  assert child.poll()is None,text[-1500:]
 if '--reze'in sys.argv:
  assert '日本語 한국어 हिन्दी العربية'in text and 'Artist'in text,text[-3000:];print('Reze native text receives title and artist');sys.exit(0)
 assert'MEDIA_TITLE:日本語 한국어 हिन्दी العربية'in text,text[-3000:];assert'MEDIA_TIME:30:180'in text,text[-3000:];assert'ReferenceError'not in text;text='initial_metadata_and_seconds_and_enum';(out/'results.json').write_text(json.dumps({text:True}));print(text,True)
finally:
 if child and child.poll()is None:child.terminate();child.wait(10)
 service.terminate();service.wait(5)
