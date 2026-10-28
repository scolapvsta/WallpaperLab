#!/usr/bin/env python3
"""Exercise config persistence, conflicts and rollback against a fake compositor."""
import json, os, pathlib, subprocess, sys, tempfile
app=str(pathlib.Path(sys.argv[1]).resolve())
runtime=pathlib.Path(app).parent
# Installed native binary needs its private transitive libraries even with a temporary HOME.
if (runtime/'deps').exists():
    os.environ['LD_LIBRARY_PATH']=os.pathsep.join(str(runtime/p) for p in ['.','lib','deps'])+os.pathsep+os.environ.get('LD_LIBRARY_PATH','')
with tempfile.TemporaryDirectory(prefix='wallpaper-lab-shortcuts-') as tmp:
    root=pathlib.Path(tmp); home=root/'home'; config=home/'.config'; state=home/'.local/state'; bin=root/'bin'
    bin.mkdir();(home/'.local/bin').mkdir(parents=True);(config/'hypr').mkdir(parents=True)
    (home/'.local/bin/wallpaper-lab').write_text('placeholder')
    main=config/'hypr/hyprland.lua';original='-- unrelated user configuration\n';main.write_text(original)
    fake=bin/'hyprctl'
    fake.write_text('''#!/usr/bin/env python3
import sys,os,json,pathlib
args=sys.argv[1:];root=pathlib.Path(os.environ['XDG_CONFIG_HOME'])
if args[0]=='systeminfo':print('configProvider: lua')
elif args[0]=='monitors':print('[]')
elif args[0]=='reload':print('ok')
elif args[0]=='binds':
 f=root/'wallpaper-lab/shortcut.lua';text=f.read_text() if f.exists() else ''
 result=[]
 if os.environ.get('CONFLICT'):result=[{'modmask':64,'key':'Tab','description':'Another app'}]
 elif 'hl.bind' in text and not os.environ.get('FAIL_BIND'):result=[{'modmask':64,'key':'Tab','description':'Wallpaper Lab carousel'}]+[{'modmask':0,'key':k,'description':'Wallpaper Lab carousel release '+k,'release':True,'non_consuming':True} for k in ['Super_L','Super_R']]
 print(json.dumps(result))
''');fake.chmod(0o755)
    env=os.environ|{'HOME':str(home),'XDG_CONFIG_HOME':str(config),'XDG_STATE_HOME':str(state),'PATH':str(bin)+os.pathsep+os.environ['PATH']}
    def run(*args,extra=None,ok=True):
        p=subprocess.run([app,*args],env=env|(extra or {}),text=True,capture_output=True)
        assert (p.returncode==0)==ok,(args,p.stdout,p.stderr)
        return p
    run('--help')  # Fail early if the binary cannot start; failures below must be behavioral.
    run('--shortcut','SUPER+Tab',extra={'CONFLICT':'1'},ok=False)
    assert main.read_text()==original and not (config/'wallpaper-lab/shortcut.lua').exists()
    run('--shortcut','SUPER+Tab',extra={'FAIL_BIND':'1'},ok=False)
    assert main.read_text()==original and not (config/'wallpaper-lab/shortcut.lua').exists()
    run('--shortcut','SUPER+Tab')
    assert 'dofile' in main.read_text()
    assert json.loads((config/'wallpaper-lab/shortcut.json').read_text())['enabled']
    assert list((state/'wallpaper-lab/backups').glob('*/hyprland.lua'))
    run('--shortcut','SUPER+Tab');assert main.read_text().count('dofile')==1
    run('--shortcut-off')
    assert not json.loads((config/'wallpaper-lab/shortcut.json').read_text())['enabled']
    assert 'hl.bind' not in (config/'wallpaper-lab/shortcut.lua').read_text()
print('Shortcut conflict detection, backup, persistence, disable and failed-reload rollback passed')
