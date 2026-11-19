"""Opt-in pixel comparison of one virtual canvas and multiple SDDM views."""
import argparse,copy,importlib.util,json,tempfile,subprocess
from pathlib import Path
from PIL import Image,ImageChops,ImageStat

p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
spec=importlib.util.spec_from_file_location('sddm_helper',a.app.resolve().parent/'sddm_integration.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
project=out/'library/image';project.mkdir(parents=True,exist_ok=True)
image=Image.new('RGB',(1600,600));image.putdata([(int(x*255/1599),int(y*255/599),60 if x<800 else 200)for y in range(600)for x in range(1600)]);image.save(project/'media.png')
(project/'project.json').write_text(json.dumps(dict(type='web',lab_origin='local',lab_media_file='media.png')))
request=dict(config=str(out/'config'),state=str(out/'state'),roots=[str(project.parent)])
candidate=dict(enabled=True,wallpaper=dict(source=str(project),display_mode='span',profiles={str(project):dict(brightness=.1,zoom=1.2,x=.3)}),ui=dict(preset='astronaut'))
results={}
def capture(name,screens,form=False):
    i=m.Integration(request|dict(screens=screens))
    with tempfile.TemporaryDirectory(prefix='span-',dir=out)as temporary:
        stage=Path(temporary);i.build_theme(copy.deepcopy(candidate),stage)
        if not form:
            (stage/'ThemeMain.qml').write_text('import QtQuick\nimport "."\nLabBackground { property rect labScreenRect; property rect labDesktopRect; screenRect:labScreenRect; desktopRect:labDesktopRect; source: Qt.resolvedUrl(config.Background) }\n')
        i.capture(stage,out/(name+'.png'))
    return Image.open(out/(name+'.png')).convert('RGB')
def error(a,b):return sum(ImageStat.Stat(ImageChops.difference(a,b)).mean)/3
single=[dict(name='canvas',x=-800,y=0,width=1600,height=600)]
dual=[dict(name='left',x=-800,y=0,width=800,height=600),dict(name='right',x=0,y=0,width=800,height=600)]
whole=capture('whole-canvas',single);split=capture('two-monitors',dual)
results['two_monitor_mean_error']=error(whole,split);assert results['two_monitor_mean_error']<1.5,results
assert split.getpixel((100,300))!=split.getpixel((1100,300));results['different_portions_not_mirrored']=True
offset=[dict(name='left',x=-800,y=0,width=800,height=600),dict(name='right',x=0,y=-200,width=800,height=800)]
offset_image=capture('offset-monitors',offset);assert offset_image.getpixel((100,30))==(0,0,0);assert max(offset_image.getpixel((1000,100)))>10;results['negative_offsets_and_unequal_sizes']=True
form=capture('combined-login',dual,True)
for box in [(0,0,640,720),(640,0,1280,720)]:
    panel=form.crop(box)
    assert error(panel,split.crop(box))>1
    assert sum(min(pixel)>230 for pixel in panel.getdata())>500
results['two_login_views']=True
# Uniform changing colours expose decoder clock drift between greeter windows.
video=project.parent/'video';video.mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-v','error','-y','-f','lavfi','-i',"nullsrc=s=1600x600:r=20,geq=r='128+70*sin(T*3)':g='100+60*cos(T*3)':b=80",'-t','4','-c:v','libx264','-pix_fmt','yuv420p',str(video/'media.mp4')],check=True)
(video/'project.json').write_text(json.dumps(dict(type='video',lab_origin='local',lab_media_file='media.mp4')))
candidate['wallpaper']=dict(source=str(video),display_mode='span')
movie=capture('panorama-video',dual)
results['video_monitor_colour_difference']=sum(abs(x-y)for x,y in zip(movie.getpixel((100,300)),movie.getpixel((1100,300))))
assert results['video_monitor_colour_difference']<30,results
(out/'panorama-video.log').read_text();assert 'TypeError' not in (out/'panorama-video.log').read_text()
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
