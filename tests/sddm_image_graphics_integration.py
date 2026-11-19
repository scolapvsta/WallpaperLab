"""Opt-in Qt/RHI pixel checks for the shared SDDM image controls and final capture."""
import argparse,copy,importlib.util,json,subprocess,tempfile
from pathlib import Path
from PIL import Image,ImageChops,ImageStat,ImageOps
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
helper=a.app.resolve().parent/'sddm_integration.py';spec=importlib.util.spec_from_file_location('sddm_helper',helper);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
library=out/'library';project=library/'image';project.mkdir(parents=True,exist_ok=True)
# Asymmetric, non-saturated quadrants expose stretching, mirroring and colour errors.
pixels=bytearray()
for y in range(360):
 for x in range(640):pixels.extend((30,70,110)if x<320 and y<180 else(170,90,40)if y<180 else(60,140,90)if x<320 else(120,50,160))
(project/'source.ppm').write_bytes(b'P6\n640 360\n255\n'+pixels)
subprocess.run(['ffmpeg','-v','error','-y','-i',str(project/'source.ppm'),str(project/'media.png')],check=True)
(project/'project.json').write_text(json.dumps(dict(title='Image control fixture',type='web',lab_origin='local',lab_media_file='media.png')))
request=dict(config=str(out/'config'),state=str(out/'state'),roots=[str(library)],action='enable')
integration=m.Integration(request)
base=dict(enabled=True,wallpaper={'source':str(project)},ui={'preset':'astronaut'})
results={}
def capture(name,values=None,form=False):
 candidate=copy.deepcopy(base)
 if values:candidate['wallpaper']['profiles']={str(project):m.validated_image(values)}
 with tempfile.TemporaryDirectory(prefix='render-',dir=out)as temp:
  stage=Path(temp);integration.build_theme(candidate,stage)
  if not form:(stage/'ThemeMain.qml').write_text('import QtQuick\nimport "."\nLabBackground { source: Qt.resolvedUrl(config.Background) }\n')
  integration.capture(stage,out/(name+'.png'))
 return Image.open(out/(name+'.png')).convert('RGB')
def difference(first,second):return sum(ImageStat.Stat(ImageChops.difference(first,second)).mean)/3
plain=capture('plain');expected=Image.open(project/'media.png').convert('RGB').resize((1280,720),Image.Resampling.BILINEAR)
results['unmodified_mean_error']=difference(plain,expected);assert results['unmodified_mean_error']<2,results
bright=capture('brightness',dict(brightness=.2));expected_bright=plain.point(lambda x:min(255,x+51))
results['brightness_mean_error']=difference(bright,expected_bright);assert results['brightness_mean_error']<2,results
mono=capture('saturation',dict(saturation=0));r,g,b=mono.split();assert ImageStat.Stat(ImageChops.difference(r,g)).mean[0]<1 and ImageStat.Stat(ImageChops.difference(g,b)).mean[0]<1
results['saturation_grayscale']=True
mirror=capture('mirror',dict(flip_x=1));results['mirror_mean_error']=difference(mirror,ImageOps.mirror(plain));assert results['mirror_mean_error']<2,results
zoom=capture('zoom',dict(zoom=.5));assert max(zoom.getpixel((40,40)))==0 and max(zoom.getpixel((640,360)))>20;results['zoom_out_black_margins']=True
rotate=capture('rotate',dict(rotation=35,hue=30,temperature=.3,gamma=1.2,vignette=.5));assert difference(plain,rotate)>15;results['additional_adjustments_render']=True
combined=capture('combined',dict(brightness=.2),form=True);assert difference(bright,combined)>5;results['final_login_composition']=True
rgba=Image.open(project/'media.png').convert('RGBA');rgba.putalpha(128);rgba.save(project/'media.png')
transparent=capture('transparent');expected_alpha=Image.new('RGBA',rgba.size,(0,0,0,255));expected_alpha.alpha_composite(rgba)
results['transparent_mean_error']=difference(transparent,expected_alpha.convert('RGB').resize((1280,720),Image.Resampling.BILINEAR));assert results['transparent_mean_error']<2,results
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
