"""Export and publish a standalone native login bundle. Never restart a live session."""
from pathlib import Path
import argparse,fcntl,hashlib,json,os,pwd,shutil,stat,subprocess,sys,tempfile,time

BASE=Path('/usr/local/lib/wallpaper-lab-login')
STATE=Path('/var/lib/wallpaper-lab-login')
CONFIG=Path('/etc/greetd/config.toml')
PAM=Path('/etc/pam.d/greetd')
KEYRING_MODULE=Path('/usr/lib/security/pam_gnome_keyring.so')
ALIAS=Path('/etc/systemd/system/display-manager.service')
RUNTIME_FILES=['wallpaper-lab','liblinux-wallpaperengine-lib.so','libcef.so','libEGL.so','libGLESv2.so','libvk_swiftshader.so','v8_context_snapshot.bin','vk_swiftshader_icd.json','chrome_100_percent.pak','chrome_200_percent.pak','resources.pak','icudtl.dat','login_session.py','login_deploy.py']
RUNTIME_DIRS=['lib','deps','locales','fonts','licenses','login-assets']

def atomic(path,data,mode=0o644):
    path.parent.mkdir(parents=True,exist_ok=True)
    temporary=path.with_name(path.name+'.new')
    with temporary.open('wb')as f:f.write(data.encode()if isinstance(data,str)else data);f.flush();os.fsync(f.fileno())
    temporary.chmod(mode);temporary.replace(path)

def digest(path):
    with path.open('rb')as file:return hashlib.file_digest(file,'sha256').hexdigest()

def checked_copy(source,destination):
    """Reject special files and escaping symlinks in exported wallpaper projects."""
    source=source.resolve();destination.mkdir(parents=True)
    for entry in source.rglob('*'):
        target=destination/entry.relative_to(source)
        if entry.is_symlink():raise ValueError('Login export does not accept project symlinks: '+str(entry))
        if entry.is_dir():target.mkdir(exist_ok=True)
        elif entry.is_file():shutil.copy2(entry,target)
        else:raise ValueError('Special file in login export: '+str(entry))

def rewrite(value,mappings):
    if isinstance(value,dict):return {k:rewrite(v,mappings)for k,v in value.items()}
    if isinstance(value,list):return [rewrite(v,mappings)for v in value]
    if isinstance(value,str):
        for source,target in mappings:
            if value==str(source)or value.startswith(str(source)+'/'):return str(target)+value[len(str(source)):]
    return value

def copy_manifest(source,target,wanted):
    """Copy through no-follow directory descriptors, hashing the bytes actually copied.

    The caller may be root while the staging directory is still owned by the
    desktop user. Never traverse it with privileged copytree/chown operations.
    """
    if not isinstance(wanted,dict)or not wanted:raise ValueError('Empty login package')
    root=os.open('/',os.O_RDONLY|os.O_DIRECTORY)
    try:
        for part in source.absolute().parts[1:]:
            child=os.open(part,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW,dir_fd=root)
            os.close(root);root=child
    except BaseException:
        os.close(root);raise
    try:target.mkdir(mode=0o700)
    except BaseException:
        os.close(root);raise
    try:
        for relative,digest in wanted.items():
            parts=relative.split('/')
            if any(p in ('','.', '..')for p in parts)or not isinstance(digest,str)or len(digest)!=64:raise ValueError('Unsafe package path')
            directory=os.dup(root)
            try:
                for part in parts[:-1]:
                    child=os.open(part,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW,dir_fd=directory)
                    os.close(directory);directory=child
                fd=os.open(parts[-1],os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK,dir_fd=directory)
                with os.fdopen(fd,'rb')as incoming:
                    if not stat.S_ISREG(os.fstat(incoming.fileno()).st_mode):raise ValueError('Special file in package')
                    destination=target.joinpath(*parts);destination.parent.mkdir(parents=True,exist_ok=True)
                    checksum=hashlib.sha256()
                    with destination.open('xb')as outgoing:
                        while chunk:=incoming.read(1024*1024):checksum.update(chunk);outgoing.write(chunk)
                    if checksum.hexdigest()!=digest:raise ValueError('Login package checksum changed')
            finally:os.close(directory)
        for p in target.rglob('*'):p.chmod(0o755 if p.is_dir()or p.name=='wallpaper-lab'else 0o644)
        target.chmod(0o755)
    except BaseException:
        shutil.rmtree(target);raise
    finally:os.close(root)

def prepare(file):
    document=json.loads(file.read_text());runtime=Path(document['renderer']).resolve().parent
    for name in ['wallpaper-lab','liblinux-wallpaperengine-lib.so','login_session.py','login_deploy.py']:
        if not(runtime/name).is_file():raise ValueError('Install the complete native Wallpaper Lab runtime before publication: '+name)
    destination=file.parent/'publication';destination.mkdir()
    release=str(time.time_ns());system=BASE/'releases'/release
    staged=destination/'bundle';(staged/'runtime').mkdir(parents=True)
    for name in RUNTIME_FILES:
        if(runtime/name).is_file():shutil.copy2(runtime/name,staged/'runtime'/name)
    for name in RUNTIME_DIRS:
        source=runtime/name
        if source.is_dir():shutil.copytree(source,staged/'runtime'/name,symlinks=False,ignore=shutil.ignore_patterns('__pycache__','libsteam_api.so'))
    project=Path(document['project']).resolve();assets=Path(document['assets']).resolve()
    checked_copy(project,staged/'project')
    # Local images/videos do not need the Steam asset tree.
    if assets==project:(staged/'assets').mkdir()
    else:checked_copy(assets,staged/'assets')
    profiles=json.loads(Path(document['profile_file']).read_text())
    profile=profiles.get(document['source'],{})
    mappings=[(project,system/'project'),(Path(document['source']).resolve(),system/'project'),(assets,system/'assets'),(runtime,system/'runtime')]
    for path in (staged/'project').rglob('*.json'):
        try:value=json.loads(path.read_text())
        except(UnicodeError,ValueError):continue
        path.write_text(json.dumps(rewrite(value,mappings),indent=2)+'\n')
    profile=rewrite(profile,mappings)
    # File properties outside the selected project must be deliberately imported first.
    for value in profile.get('properties',{}).values():
        if isinstance(value,str)and value.startswith('/')and not value.startswith(str(system)+'/'):
            raise ValueError('Import external scene property files into the project before publishing')
    # Each independent output gets its own project and profile; repeated source
    # projects may still have different placement or colour settings.
    independent={}
    for index,(output,record) in enumerate(document.get('prepared_wallpapers',{}).items()):
        folder='project-'+str(index);chosen=Path(record['project']).resolve();checked_copy(chosen,staged/folder)
        specific=[(chosen,system/folder),(Path(record['source']).resolve(),system/folder),(assets,system/'assets'),(runtime,system/'runtime')]
        for path in (staged/folder).rglob('*.json'):
            try:value=json.loads(path.read_text())
            except(UnicodeError,ValueError):continue
            path.write_text(json.dumps(rewrite(value,specific),indent=2)+'\n')
        selected=json.loads(Path(record['profile_file']).read_text()).get(record['source'],{})
        selected=rewrite(selected,specific)
        for value in selected.get('properties',{}).values():
            if isinstance(value,str)and value.startswith('/')and not value.startswith(str(system)+'/'):raise ValueError('Import external scene property files into the project before publishing')
        profile_name='profile-'+str(index)+'.json';(staged/profile_name).write_text(json.dumps({str(system/folder):selected},indent=2)+'\n')
        independent[output]=record|dict(project=str(system/folder),source=str(system/folder),assets=str(system/'assets'),profile_file=str(system/profile_name))
    original=document['source'];document=rewrite(document,mappings);document.pop('owner',None)
    document.update(renderer=str(system/'runtime/wallpaper-lab'),project=str(system/'project'),assets=str(system/'assets'),profile_file=str(system/'wallpaper_profiles.json'),source=str(system/'project'))
    document['profiles']={str(system/'project'):document.get('profiles',{}).get(original,{})}
    document['prepared_wallpapers']=independent
    document['wallpapers']={output:record['source']for output,record in independent.items()}
    document['output_profiles']={}
    bundled={'Oxanium':'oxanium/Oxanium[wght].ttf','Orbitron':'orbitron/Orbitron[wght].ttf','Rajdhani':'rajdhani/Rajdhani-SemiBold.ttf','Silkscreen':'silkscreen/Silkscreen-Regular.ttf'}
    candidate=runtime/'fonts'/bundled.get(document['font'],'missing')
    font=str(candidate)if candidate.is_file()else subprocess.check_output(['fc-match','-f','%{file}',document['font']],text=True).strip()
    if font and Path(font).is_file():
        shutil.copy2(font,staged/'font.ttf');document['font_file']=str(system/'font.ttf')
    (staged/'login.json').write_text(json.dumps(document,indent=2)+'\n')
    (staged/'wallpaper_profiles.json').write_text(json.dumps({str(system/'project'):profile},indent=2)+'\n')
    (staged/'settings.json').write_text(json.dumps({'muted':document.get('muted',True)})+'\n')
    # A solid native image remains usable if the chosen project fails at boot.
    fallback=staged/'fallback';fallback.mkdir();(fallback/'background.ppm').write_bytes(b'P6\n2 2\n255\n'+bytes([13,20,31])*4)
    (fallback/'project.json').write_text(json.dumps({'type':'video','file':'background.ppm','title':'Wallpaper Lab'}))
    safe=document|dict(project=str(system/'fallback'),assets=str(system/'assets'),title='Wallpaper Lab',mode='mirror',prepared_wallpapers={})
    (staged/'fallback.json').write_text(json.dumps(safe,indent=2)+'\n')
    hashes={str(p.relative_to(staged)):digest(p)for p in sorted(staged.rglob('*'))if p.is_file()}
    manifest={'schema':1,'release':release,'files':hashes,'required_packages':['greetd','hyprland']}
    (destination/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    result={'ok':True,'package':str(destination),'message':'Pacchetto di login pronto per l’installazione.'}
    print(json.dumps(result));return result

def install(package):
    if os.geteuid()!=0:raise PermissionError('System login installation requires administrator authentication')
    package=package.resolve();manifest=json.loads((package/'manifest.json').read_text());release=manifest['release']
    if not release.isdigit()or len(release)>24:raise ValueError('Invalid release identifier')
    source=package/'bundle';wanted=manifest['files']
    if not {'runtime/wallpaper-lab','runtime/login_session.py','runtime/login_deploy.py','login.json','fallback.json'}.issubset(wanted):raise ValueError('Incomplete login package')
    missing=[name for name in ['greetd','agreety','Hyprland','start-hyprland','hyprctl','dbus-run-session']if not shutil.which(name)]
    if missing:raise ValueError('Install the greetd and hyprland system packages first: '+', '.join(missing))
    greeter=pwd.getpwnam('greeter')
    BASE.mkdir(parents=True,exist_ok=True)
    BASE.chmod(0o755)
    with(BASE/'publication.lock').open('w')as lock:
        fcntl.flock(lock,fcntl.LOCK_EX)
        backup=BASE/'backups'/release;backup.mkdir(parents=True);backup.parent.chmod(0o700);backup.chmod(0o700)
        alias=ALIAS
        installed=BASE/'installed.json'
        previous={'display_manager':os.readlink(alias)if alias.is_symlink()else None,'current':os.readlink(BASE/'current')if(BASE/'current').is_symlink()else None,'had_config':CONFIG.exists(),'had_installed':installed.exists()}
        if CONFIG.exists():shutil.copy2(CONFIG,backup/'greetd-config.toml')
        if installed.exists():shutil.copy2(installed,backup/'installed.json')
        original_backup=json.loads(installed.read_text()).get('backup',str(backup))if previous['current']and installed.exists()else str(backup)
        (backup/'state.json').write_text(json.dumps(previous,indent=2)+'\n')
        target=BASE/'releases'/release;target.parent.mkdir(exist_ok=True);target.parent.chmod(0o755);copy_manifest(source,target,wanted)
        (target/'assets').mkdir(exist_ok=True)
        state=STATE;state.mkdir(parents=True,exist_ok=True);os.chown(state,greeter.pw_uid,greeter.pw_gid);state.chmod(0o700)
        command='/usr/bin/python3 '+str(target/'runtime/login_session.py')+' '+str(target)
        config='[terminal]\nvt = 1\n\n[default_session]\ncommand = '+json.dumps(command)+'\nuser = "greeter"\n'
        # Keep keyring unlock in PAM. The application never handles keyring secrets.
        pam_original=PAM.read_bytes()if PAM.is_file()else None
        pam_updated=pam_original
        if pam_original is not None and KEYRING_MODULE.is_file():
            lines=pam_original.decode().splitlines();active=[line for line in lines if line.strip()and not line.lstrip().startswith('#')]
            additions=[]
            if not any(line.lstrip('-').split()[0]=='auth'and 'pam_gnome_keyring.so'in line for line in active):additions.append('auth optional pam_gnome_keyring.so')
            if not any(line.lstrip('-').split()[0]=='session'and 'pam_gnome_keyring.so'in line for line in active):additions.append('session optional pam_gnome_keyring.so auto_start')
            if additions:pam_updated=pam_original.rstrip()+b'\n\n# Wallpaper Lab: unlock the login keyring through PAM.\n'+('\n'.join(additions)+'\n').encode()
        if pam_updated!=pam_original:
            for snapshot in [backup,Path(original_backup)]:
                if not(snapshot/'greetd-pam.original').exists():(snapshot/'greetd-pam.original').write_bytes(pam_original)
                (snapshot/'greetd-pam.applied.sha256').write_text(hashlib.sha256(pam_updated).hexdigest())
        try:
            if pam_updated!=pam_original:atomic(PAM,pam_updated)
            atomic(CONFIG,config)
            new=BASE/'current.new';new.unlink(missing_ok=True);new.symlink_to(target);new.replace(BASE/'current')
            # Enable for the next boot only; never stop the user's active display manager.
            subprocess.run(['systemctl','enable','--force','greetd.service'],check=True,capture_output=True)
        except BaseException:
            restore(backup);raise
        atomic(installed,json.dumps({'release':release,'backup':original_backup,'previous':str(backup),'bundle':str(target),'config_sha256':hashlib.sha256(config.encode()).hexdigest()},indent=2)+'\n')
    print(json.dumps({'ok':True,'installed':str(target),'backup':original_backup,'next_boot':True}))

def restore(backup):
    previous=json.loads((backup/'state.json').read_text())
    pam_backup=backup/'greetd-pam.original';pam_hash=backup/'greetd-pam.applied.sha256'
    if pam_backup.exists():
        if pam_hash.exists()and PAM.read_bytes()!=pam_backup.read_bytes()and digest(PAM)!=pam_hash.read_text():raise ValueError('PAM configuration changed outside Wallpaper Lab; restore it manually')
        atomic(PAM,pam_backup.read_bytes())
    if previous['had_config']:atomic(CONFIG,(backup/'greetd-config.toml').read_bytes())
    else:CONFIG.unlink(missing_ok=True)
    current=BASE/'current';current.unlink(missing_ok=True)
    if previous['current']:current.symlink_to(previous['current'])
    if previous.get('had_installed'):atomic(BASE/'installed.json',(backup/'installed.json').read_bytes())
    else:(BASE/'installed.json').unlink(missing_ok=True)
    alias=ALIAS;alias.unlink(missing_ok=True)
    if previous['display_manager']:alias.symlink_to(previous['display_manager'])
    subprocess.run(['systemctl','daemon-reload'],check=True)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('action',choices=['prepare','install','rollback']);p.add_argument('path',type=Path);a=p.parse_args()
    if a.action=='prepare':prepare(a.path)
    elif a.action=='install':install(a.path)
    else:
        if os.geteuid()!=0:raise PermissionError('Rollback requires administrator authentication')
        path=a.path.resolve()
        if path.parent!=BASE/'backups':raise ValueError('Unknown login backup')
        with(BASE/'publication.lock').open('w')as lock:
            fcntl.flock(lock,fcntl.LOCK_EX)
            installed=BASE/'installed.json'
            if installed.exists():
                expected=json.loads(installed.read_text()).get('config_sha256')
                if expected and(not CONFIG.is_file()or digest(CONFIG)!=expected):raise ValueError('Login configuration changed outside Wallpaper Lab; restore the backup manually')
            restore(path)
        print(json.dumps({'ok':True,'restored':str(path)}))
if __name__=='__main__':
    try:main()
    except Exception as e:print(json.dumps({'ok':False,'error':str(e)}));sys.exit(1)
