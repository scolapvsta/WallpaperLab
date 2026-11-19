#!/usr/bin/env python3
"""Translate the pinned Astronaut designs into editable native UI documents.

Geometry follows Main.qml, LoginForm.qml and Input.qml at 1920x1080.
Font line boxes were checked against an offscreen Qt reference at 96 DPI.
No SDDM code, backgrounds, or restricted fonts are loaded by the application.
"""
import configparser
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'vendor/astronaut-design'
DEST = ROOT / 'assets/login/astronaut'
PIN = 'abb3163c724935af888ba5ea9ac0c4f22afd8048'
# Clock/date/header line heights; system button center offsets from form center.
METRICS = {
    'astronaut': (213, 48, 0, [-122, -40, 37, 123]),
    'black_hole': (180, 40, 0, [-124, -41, 38, 125]),
    'cyberpunk': (144, 32, 48, [-120, -40, 40, 120]),
    'hyprland_kath': (144, 32, 0, [-120, -40, 40, 120]),
    'jake_the_dog': (193, 43, 0, [-116, -38, 36, 116]),
    'japanese_aesthetic': (173, 39, 0, [-129, -42, 39, 130]),
    'pixel_sakura': (180, 29, 0, [-120, -40, 40, 120]),
    'pixel_sakura_static': (180, 29, 0, [-120, -40, 40, 120]),
    'post-apocalyptic_hacker': (216, 48, 0, [-120, -40, 40, 120]),
    'purple_leaves': (213, 48, 0, [-122, -40, 37, 123]),
}

def generate():
    actual = subprocess.check_output(['git', '-C', str(SOURCE), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != PIN:
        raise SystemExit('Review the upstream changes before changing the pinned designs.')
    for filename in sorted((SOURCE / 'Themes').glob('*.conf')):
        parser = configparser.ConfigParser(interpolation=None)
        parser.optionxform = str
        parser.read(filename)
        c = {k: v.strip('"') for k, v in parser['General'].items()}
        yes = lambda key: c.get(key) == 'true'
        pt = int(c['FontSize'] or 13)
        px = round(pt * 4 / 3)
        padding = int(c.get('ScreenPadding') or 0)
        fw, fh = (1920 - padding * 2) / 2.5, 1080 - padding * 2
        fx = {'left': padding, 'center': (1920-fw)/2, 'right': 1920-padding-fw}[c['FormPosition']]
        th, dh, hh, centers = METRICS[filename.stem]
        form = {'x': fx / 1920, 'y': padding / 1080, 'width': fw / 1920, 'height': fh / 1080}
        common = dict(enabled=True, font=c['Font'], effect='none', background_opacity=0, border_width=0,
                      opacity=1, align='center', bold=c['Font']=='Open Sans', italic=False, underline=False, tracking=0)
        def widget(y, h, size, color):
            return dict(common, x=fx/1920, y=y/1080, width=fw/1920, height=h/1080, size=size, color=color)
        clock = widget(153+hh, th, pt*12, c['TimeTextColor'])
        clock.update(format='24', seconds=False)
        date = widget(153+hh+th, dh, round(pt*8/3), c['DateTextColor'])
        date['format'] = {'dddd d MMMM': 'astronaut_long', 'dddd d': 'astronaut_day', 'M/dd 2077': 'cyberpunk'}[c['DateFormat']]
        date['uppercase'] = False
        login = dict(common, **form, size=px, color=c['LoginFieldTextColor'], layout='astronaut',
                     radius=0, background=c['FormBackgroundColor'],
                     field_color=c['LoginFieldBackgroundColor'], field_opacity=.2,
                     field_radius=int(c['RoundCorners'] or 0), focus_width=0,
                     button_color=c['LoginButtonBackgroundColor'], button_text_color=c['LoginButtonTextColor'],
                     button_text_size=1, button_bold=c['Font']=='Open Sans', button_effect='none', labels=False,
                     welcome_enabled=bool(c['HeaderText']), welcome_text=c['HeaderText'], dynamic_welcome=False,
                     button_label=c.get('TranslateLogin') or 'Login', user_label=c.get('TranslatePlaceholderUsername') or 'User name',
                     password_label=c.get('TranslatePlaceholderPassword') or 'Password', session_label='Session',
                     field_height=pt*3/fh, button_height=pt*3/fh, dot_size=px*.12)
        login['background_opacity'] = (.3 if yes('PartialBlur') else 1) if yes('HaveFormBackground') else 0
        login['astronaut'] = dict(content_width=.5, field_y=(541-padding+2*pt+.75*pt)/fh,
                                  row_gap=pt*1.5/fh, button_y=(541-padding+14*pt)/fh,
                                  session_y=(998-padding)/fh, keyboard_y=(1023-padding)/fh,
                                  actions_y=(886.5-padding)/fh, action_centers=[v/fw for v in centers],
                                  header_y=(153-padding)/fh, header_height=max(1,hh)/fh, header_size=px*3,
                                  button_visible=not yes('HideLoginButton'), system_buttons=not yes('HideSystemButtons'),
                                  virtual_keyboard=not yes('HideVirtualKeyboard'), blur=32 if yes('PartialBlur') else 0,
                                  password_color=c['PasswordFieldTextColor'], password_background=c['PasswordFieldBackgroundColor'],
                                  placeholder_color=c['PlaceholderTextColor'], user_icon_color=c['UserIconColor'], password_icon_color=c['PasswordIconColor'],
                                  header_color=c['HeaderTextColor'], system_color=c['SystemButtonsIconsColor'], session_color=c['SessionButtonTextColor'],
                                  keyboard_color=c['VirtualKeyboardButtonTextColor'], warning_color=c['WarningColor'],
                                  dropdown_color=c['DropdownTextColor'], dropdown_background=c['DropdownBackgroundColor'],
                                  dropdown_selected=c['DropdownSelectedBackgroundColor'], highlight=c['HighlightBorderColor'] if c['HighlightBorderColor']!='transparent' else '#000000',
                                  highlight_opacity=0 if c['HighlightBorderColor']=='transparent' else .2)
        if filename.stem=='black_hole':
            # Qt padding shifts the input and lower controls by four/five pixels.
            login['astronaut'].update(field_y=(573-padding)/fh, button_y=(708-padding)/fh,
                                     session_y=(994-padding)/fh, keyboard_y=(1019-padding)/fh)
        data = dict(font=c['Font'], accent=c['LoginButtonBackgroundColor'], grid=dict(visible=False,snap=True,step=.025), widgets=dict(clock=clock,date=date,login=login))
        preset = dict(format='wallpaper-lab-login-preset',version=1,kind='ui',name='Astronaut · '+filename.stem.replace('_',' ').replace('-',' ').title(),data=data,
                      provenance=dict(project='sddm-astronaut-theme',author='Keyitdev',commit=PIN,source='Themes/'+filename.name,license='GPL-3.0-or-later'))
        (DEST/'presets'/f'{filename.stem}.json').write_text(json.dumps(preset,indent=2)+'\n')
    for icon in ['User','Password','Password2','Shutdown','Reboot','Suspend','Hibernate']:
        subprocess.run(['rsvg-convert','-w','128','-h','128',str(SOURCE/'Assets'/f'{icon}.svg'),'-o',str(DEST/'icons'/f'{icon}.png')],check=True)

if __name__ == '__main__':
    generate()
