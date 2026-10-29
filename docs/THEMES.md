# Themes, fonts and switchers

| Theme | Mode | Palettes | Initial suggested font | Included backgrounds |
| --- | --- | ---: | --- | ---: |
| Default | Dark | 3 | System Sans | 4 |
| Strawberry core | Light | 3 | System Sans | 4 |
| 2077 | Dark | 11 | Oxanium | 4 |
| Pixel Theme | Dark | 4 | Silkscreen | 4 |

**Themes** and **Color Palettes** are dropdowns. Each theme has its own palettes,
background and suggested-font selection. Mode is determined by the theme. Fresh
installations start with Default/Graphite. Existing Strawberry and 2077 palette
IDs are preserved; superseded presets are outside the active catalog.

## Default

Soft surfaces and understated borders. Palettes: **Graphite**, **Blue Slate** and
**Forest Ink**. Backgrounds: **Quiet Gradient**, **Blueprint**, **Contour** and
**Night Dots**. Suggested fonts include Sans, Adwaita Sans and Noto Sans, with
system fallback when needed.

## Strawberry core

Palettes: **Strawberry Milk**, **Raspberry Ribbon** and **Vanilla Shortcake**.
Backgrounds: **Strawberry Gingham** (checks), **Ribbon Atelier** (diagonal ribbons
and bows), **Strawberry Garden** (strawberries, flowers and a rabbit) and
**Tea Party Lace** (lace bands and doilies).

Cookie-shaped controls use smooth capsules, pink hover feedback, darker pressed
states and contained icons. Decorations are drawn in application code.

## 2077

Palettes: **Violet Afterglow**, **Pink Voltage**, **Electric Blue**, **Night Drive**,
**Hazard Yellow**, **Crimson Protocol**, **Toxic Lime**, **Amber Circuit**,
**Ice Chrome**, **Ultraviolet** and **Copper Sunset**.

Backgrounds: **Neon District**, **Afterlife Alley**, **Binary Rain** and
**Edge Signal**. Neon skylines, rain, LEDs and binary details accompany the
WALLPAPER LAB / CONTROL DECK header. The 2077 mark uses chromatic separation and
glitch bands for about 0.32 seconds every 1.8 seconds. Reduced motion stops these
animations. Small side accents appear on text buttons.

## Pixel Theme

Stepped panel shadows, original adventurer/slime/robot/ghost sprites, clouds,
torches and stars. Palettes: **Arcade Berry**, **Dungeon Moss**, **Coin Rush** and
**CRT Ocean**. Backgrounds: **Overworld**, **Dungeon**, **Arcade** and **Star Quest**.
Animations respect reduced motion. Sprites are original application drawings.
Silkscreen is bundled under SIL OFL 1.1; alternative suggested fonts include
Oxanium and DejaVu Sans Mono.

## Suggested Theme Fonts and General Fonts

**Suggested Theme Fonts** selects a font for the current theme and remembers it
per theme. **General Fonts** stores an independent selection and has an adjacent
checkbox. Enabling it overrides the suggested font throughout the UI, including
headers and the switcher; disabling it immediately restores the theme font.
The stored general selection remains available while the override is disabled.
Fonts are registered only within the application process.

2077 suggests Oxanium, Rajdhani and Orbitron. Default and Strawberry suggest Sans,
Adwaita Sans and Noto Sans. Bundled fonts come from Google Fonts at the commit in
`assets/fonts/manifest.json`, with SHA-256 hashes and OFL notices. Missing fonts
and uncovered scripts use Fontconfig/Pango fallback.

- [Silkscreen](https://github.com/google/fonts/tree/main/ofl/silkscreen)
- [Oxanium](https://github.com/google/fonts/tree/main/ofl/oxanium)
- [Orbitron](https://github.com/google/fonts/tree/main/ofl/orbitron)
- [Rajdhani](https://github.com/google/fonts/tree/main/ofl/rajdhani)
- [Edgerunners visual reference](https://www.cyberpunk.net/en/edgerunners)

## Themed switchers

Grid, River, Vertical and Wave use the selected theme's colors, borders and font
in both the in-app preview and the real popup. The switcher uses wallpaper
thumbnails; confirming a different wallpaper starts full desktop rendering.
The main window's imported background is separate from switcher content.

## Importing an app background

Choose one of four included backgrounds or **Load from PC**. Imported files are
copied as static PNGs, up to 2048 × 2048, into the XDG data directory under
`wallpaper-lab/app-backgrounds/`. The original can then be moved or removed.
**Restore theme** selects the theme's first included background. Invalid images
retain the previous selection.

The imported image is shared across themes and is separate from the desktop
wallpaper catalog. Thumbnails use a canonical 1280 × 720 composition before
scaling. GIFs use their first frame. Import requires Zenity/KDialog and FFmpeg.
Imported backgrounds are static and center-cropped. Native decorative scenes are
stylized drawings; they do not provide 3D rendering.

## Historical collections (0.18)

Before the theme redesign, collections grouped 29 palettes: Classics (5), Neon
(6), Pastel (9), Glass (3), Pixel (3) and LED/RGB (3). These historical options
are not the current theme menu. Palette selection remained independent of UI
transparency, and reduced motion stopped decorative effects.

Historical visual references:

- [Pinterest Palette 2026](https://newsroom.pinterest.com/pdf/pinterest-palette-2026-pdf/):
  cool tones, vivid accents and the Plum Noir surface reference `#351E28`.
- [Dark Neon UI](https://in.pinterest.com/pin/ui_neonline--353954851938453965/):
  dark surfaces and luminous outlines.
- [Lavender Mint](https://in.pinterest.com/pin/696369161170866719/): pastel pairings.
- [Retro Pixel Art Computer](https://de.pinterest.com/pin/retro-pixel-art-computer--704743041686782481/):
  geometric retro panels.

These are palette and visual references. Effects, panels and shaders are
implemented in application code; reference images are not shipped. Some Pinterest
references may require login beyond their indexed preview. License and font
attributions are in [THIRD_PARTY.md](../THIRD_PARTY.md).
