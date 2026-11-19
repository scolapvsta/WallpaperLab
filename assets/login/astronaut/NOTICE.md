# Astronaut native UI designs

Presets, layout adaptation and rasterized icons derive from
[sddm-astronaut-theme](https://github.com/Keyitdev/sddm-astronaut-theme),
commit `abb3163c724935af888ba5ea9ac0c4f22afd8048`.

Copyright (C) 2022–2025 Keyitdev. Based on Marian Arlt's
[sddm-sugar-dark](https://github.com/MarianArlt/sddm-sugar-dark).
Distributed under GPL-3.0-or-later; see COPYING. Native adaptation by
Wallpaper Lab contributors, 2026, under the same license.

The design reference is pinned at `vendor/astronaut-design`. Regenerate the
preset JSON and icon masks with `python scripts/import-astronaut-presets.py`
(requires rsvg-convert). Production uses native Cairo/Pango/OpenGL rendering;
it neither launches SDDM nor loads QML. Original wallpapers are not bundled.

Open Sans (SIL OFL 1.1), Electroharmonix and Fragile Bombers Attack (CC0 releases
identified in the embedded font notices) are in `assets/fonts/astronaut` with
license notices. ESPACION, KogniGear, Thunderman, Pixelon and ArcadeClassic are
referenced by family name and used when installed by the user. They are not
redistributed: their embedded notices do not all grant redistribution. Missing
families fall back through Fontconfig. The native editor reports substitutions.

Pixel Sakura and Pixel Sakura Static intentionally have identical UI settings;
the upstream difference is their background, outside the scope of a UI preset.
