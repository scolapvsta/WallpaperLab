#!/usr/bin/env python3
"""Audit UI calls, dynamic label tables, stored notices and backend diagnostics."""
import json, pathlib, re
root = pathlib.Path(__file__).resolve().parents[1]
catalog = json.loads((root/'src/translations.json').read_text())
rules = json.loads((root/'src/ui_messages.json').read_text())
keys = set(rules.values())
literal = re.compile(r'"(?:[^"\\]|\\.)*"')
tokenizer = re.compile(r'"(?:[^"\\]|\\.)*"|//[^\n]*|/\*[\s\S]*?\*/|[A-Za-z_]\w*|[^\s]')
# Brand names, visual wordmarks, key names and protocol/unit notation stay literal.
fixed = {'Wallpaper Lab','Caelestia','awww','post-hook.log','s','Steam Workshop','Steam','Workshop','Ctrl + L','AND / OR',
         'strawberry core','DESKTOP // CONTROL DECK'}
raw_ui = []
def strings(tokens):
    return [json.loads(t) for t in tokens if literal.fullmatch(t)]
def calls(tokens):
    for i, name in enumerate(tokens[:-1]):
        if tokens[i+1] != '(' or not re.fullmatch(r'[A-Za-z_]\w*', name): continue
        depth=0; args=[]; arg=[]
        for t in tokens[i+2:]:
            if t==')' and depth==0:
                args.append(arg); yield name,args; break
            if t==',' and depth==0: args.append(arg);arg=[];continue
            if t in ('(', '{', '['):depth+=1
            if t in (')', '}', ']'):depth-=1
            arg.append(t)
for file in (root/'src').iterdir():
    if file.suffix not in ('.h','.hpp','.cpp','.c') or file.name in ('gif.c','gif.h'):continue
    text=file.read_text()
    tokens=[t for t in tokenizer.findall(text) if not t.startswith(('//','/*'))]
    for name,args in calls(tokens):
        if name=='lab_tr':
            for key in strings(args[0]):
                if key not in fixed|{'span','mirror','muted'} and not re.fullmatch(r'[a-z]+_[a-z_]+',key):keys.add(key)
        if name=='snprintf' and args and args[0]==['notice']:
            for key in strings(args[2]):
                if key not in ('','%s'):keys.add(key)
            # Completion notices choose a canonical message after "%s".
            if len(args)>3 and '?' in args[3]:keys.update(strings(args[3]))
        index={'UiText':0,'TextLines':0,'Button':1,'DangerButton':1,'Choice':4,'BoolChoice':4}.get(name)
        if index is not None and len(args)>index:
            label=args[index]
            if len(label)==1 and literal.fullmatch(label[0]):
                key=json.loads(label[0])
                if key and re.search(r'[A-Za-zÀ-ž]',key) and key not in fixed:raw_ui.append((file.name,key))
    # Every literal diagnostic must have an explicit localized route, even if
    # it later receives an OS message, a file path or a Steam error code.
    for match in re.finditer(r'(?:runtime_error\((?:std::string\()?|(?:reason|\["error"\])\s*=\s*)("(?:[^"\\]|\\.)*")',text):
        key=json.loads(match[1])
        assert not key or key in catalog or key in rules,(file.name,'unregistered diagnostic',key)
# Explicit tables feed lab_tr(variable), not lab_tr("literal").
for file,table in [('transition_ui.h','transitionNames'),('wallpaper_controls.h','names'),
                   ('wallpaper_controls.h','flipNames'),('wallpaper_controls.h','labels'),
                   ('resolution_picker.h','names'),('workshop_ui.h','sortNames')]:
    text=(root/'src'/file).read_text();match=re.search(r'\b'+table+r'\[\]\s*=\s*\{([^}]+)\}',text)
    assert match,(file,table)
    keys.update(json.loads(s) for s in literal.findall(match[1]))
text=(root/'src/project_properties.hpp').read_text().split('labels=')[1].split(';')[0]
keys.update(b for a,b in re.findall(r'\{"([^"]+)","([^"]+)"\}',text))
assert not raw_ui,('untranslated UI literals',raw_ui)
assert not keys-catalog.keys(),sorted(keys-catalog.keys())
for key,row in catalog.items():
    assert len(row)==21 and all(isinstance(v,str) and v for v in row),key
    if len(set(row))==1:assert re.fullmatch(r'\d+ × \d+',row[0]),('untranslated locale row',key)
    if len(key.split())>3:assert key not in row,('Italian sentence leaked to another locale',key)
print(f'{len(catalog)} catalog entries, {len(rules)} diagnostic routes and {len(keys)} source keys audited across 22 locales')
