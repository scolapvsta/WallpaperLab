#!/usr/bin/env python3
"""Chromatic Material You palettes, optional component preparation, no desktop changes."""
import argparse,json,os,pathlib,subprocess,sys
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--helper',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);env=os.environ|{'XDG_DATA_HOME':str(out/'data'),'XDG_STATE_HOME':str(out/'state'),'XDG_CACHE_HOME':str(out/'cache')};results={}
for name,color in [('red','#df3455'),('blue','#246dc0')]:Image.new('RGB',(512,288),color).save(out/(name+'.png'))
ready=json.loads(subprocess.check_output([sys.executable,str(a.helper.resolve()),'--prepare'],env=env,text=True));assert pathlib.Path(ready['python']).exists();assert(out/'data/wallpaper-lab/integrations/caelestia/presets.json').is_file()
for mode in ['dark','light']:
 for variant in ['vibrant','tonalspot','expressive','fidelity']:
  palettes=[]
  for name in ['red','blue']:
   palette=json.loads(subprocess.check_output([sys.executable,str(a.helper.resolve()),'--variant',variant,'--mode',mode,str(out/(name+'.png'))],env=env,text=True));assert palette['mode']==mode and palette['variant']==variant;assert len(palette['colours'])>40;surface=palette['colours']['surface'];rgb=[int(surface[n:n+2],16)for n in [0,2,4]];assert max(rgb)-min(rgb)>15;palettes.append(palette)
  assert palettes[0]['colours']['primary']!=palettes[1]['colours']['primary'];assert palettes[0]['colours']['surface']!=palettes[1]['colours']['surface'];results[mode+'/'+variant]=True
assert not (out/'state/caelestia').exists();results['prepare_installs_optional_presets_only']=True;(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))
