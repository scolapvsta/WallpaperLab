#!/usr/bin/env python3
"""UI literals must have a full locale row; creator content remains untranslated."""
import json,pathlib,re
root=pathlib.Path(__file__).resolve().parents[1];catalog=json.loads((root/'src/translations.json').read_text());keys=set()
for file in (root/'src').glob('*.h'):
 for literal in re.findall(r'lab_tr\(("(?:[^"\\]|\\.)*")\)',file.read_text()):keys.add(json.loads(literal))
# Names are selected dynamically by the transition dropdown and preview.
keys.update(['Nessuna','Dissolvenza incrociata','Dissolvenza al nero','Zoom avanti','Zoom indietro','Scorrimento','Tendina','Apertura dal puntatore','Chiusura e apertura dal puntatore'])
assert not keys-catalog.keys(),sorted(keys-catalog.keys())
for key in keys:assert len(catalog[key])==21 and all(catalog[key]),key
# Repeated English fallback rows previously passed structural validation.
# Numeric resolution labels are deliberately identical in every language.
for key,row in catalog.items():
 if len(set(row))==1:assert re.fullmatch(r'\d+ × \d+',row[0]),('untranslated locale row',key)
print(f'{len(keys)} UI keys covered in all 22 locales')
