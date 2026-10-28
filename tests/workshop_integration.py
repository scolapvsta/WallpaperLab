#!/usr/bin/env python3
"""Opt-in Steam read-only queries; downloads occur only with --download-id."""
import argparse,json,pathlib,subprocess,os
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);p.add_argument('--download-id');a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);env=os.environ|{'XDG_STATE_HOME':str(out/'state'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','deps','lib'])};results={}
def query(name,q):
 r=subprocess.run([str(app),'--workshop-search',json.dumps(q)],env=env,capture_output=True,text=True,timeout=75);(out/(name+'.json')).write_text(r.stdout);(out/(name+'.log')).write_text(r.stderr);j=json.loads(r.stdout);assert r.returncode==0,(name,j['error']);return j
and_=query('tags-and',{'text':'minimal','tags':['Scene','Video'],'any':False,'sort':0});or_=query('tags-or',{'text':'minimal','tags':['Scene','Video'],'any':True,'sort':0});assert and_['total']==0;assert or_['total']>0 and or_['items'];assert all('Scene'in i['tags']or'Video'in i['tags']for i in or_['items']);results['and_vs_or']=[and_['total'],or_['total']]
resolution=query('resolution',{'text':'minimal','tags':['Scene','1920 x 1080'],'sort':0});assert resolution['items']and all('Scene'in i['tags']and'1920 x 1080'in i['tags']for i in resolution['items']);results['type_and_resolution']=True
owner='76561198399626031';author=query('author-and-name',{'author':owner,'text':'Frieren','tags':['Scene'],'any':False});assert author['items']and all(i['owner']==owner for i in author['items']);assert any(i['id']=='3100052627'for i in author['items']);results['author_and_text_across_all_author_pages']=True
preset=query('preset-dependencies',{'text':'1806366520'});assert preset['items'][0]['dependencies']==['893418273'];results['preset_dependencies']=preset['items'][0]['dependencies']
detail=query('direct-url',{'text':'https://steamcommunity.com/sharedfiles/filedetails/?id=1818234419'});assert len(detail['items'])==1 and detail['items'][0]['id']=='1818234419';assert detail['items'][0]['description']and detail['items'][0]['owner'];results['direct_id_url_and_details']=True
page2=query('page-2',{'text':'minimal','sort':0,'page':2});assert page2['page']==2 and page2['items'];results['pagination']=True
# A foreign app ID is rejected before subscribing or downloading anything.
r=subprocess.run([str(app),'--workshop-download','1840'],env=env,capture_output=True,text=True,timeout=45);j=json.loads(r.stdout);assert r.returncode!=0 and j['downloads']['1840']['status']=='error';results['invalid_item_rejected']=True;(out/'invalid.json').write_text(r.stdout)
if a.download_id:
 r=subprocess.run([str(app),'--workshop-download',a.download_id],env=env,capture_output=True,text=True,timeout=180);(out/'download.json').write_text(r.stdout);(out/'download.log').write_text(r.stderr);j=json.loads(r.stdout);assert r.returncode==0,(r.returncode,j);assert j['downloads'][a.download_id]['status']=='installed';results['real_download']=j['downloads'][a.download_id]
(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))
