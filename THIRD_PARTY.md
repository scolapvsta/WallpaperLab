# Provenienza e licenze

Wallpaper Lab è un progetto derivato, distribuito con GPL-3.0. Il nome è provvisorio.

- **Almamu/linux-wallpaperengine**, GPL-3.0: renderer nativo, parser PKG/TEX,
  scene, video, web e integrazione desktop. Il commit è fissato nel submodule
  `vendor/engine`. Le modifiche locali sono in `patches/engine-integration.patch` e
  `patches/engine-build-paths.patch` (quoting dei percorsi CMake) e
  `patches/engine-audio-web.patch` (audio stereo, bridge web/SceneScript, lifecycle
  CEF e compatibilità OpenGL delle copertine).
  La calibrazione dei panorami e raccolta in `patches/engine-display-layout.patch`.
  Il posizionamento per progetto è raccolto in `patches/engine-wallpaper-placement.patch`.
  Aggiornamenti live e vista ridotta del renderer sono in `patches/engine-live-controls.patch`.
  Parsing delle scene, shader di clipping e interazione SceneScript sono in
  `patches/engine-scene-interaction.patch`.
- **danihek/hellpaper**, copyright 2025 danihek, e fork **Belial02/hellpaper**, MIT:
  base del selettore Raylib, shader, quattro layout e animazioni. `src/selector.c`
  è la versione modificata per questa applicazione; l'originale è fissato in
  `vendor/hellpaper`. Avviso MIT completo in `assets/LICENSE.hellpaper`.
- **raylib**, Ramon Santamaria e contributori, zlib/libpng: UI e immagini.
  Versione 5.5 fissata in `vendor/raylib`; licenza in `assets/LICENSE.raylib`.
  Il decoder GIF usa stb_image vendorizzato da Raylib, con i relativi avvisi nel sorgente.
- **FreeType** di sistema: rasterizzazione del fallback per i titoli nei font TTC.
  Fontconfig risolve i font installati e registra le famiglie incluse soltanto
  nel processo dell’app.
- **Chromium Embedded Framework** e dipendenze del renderer mantengono le proprie
  licenze e attribuzioni upstream. Le risorse CEF sono scaricate dal build upstream;
  la distribuzione locale include i suoi avvisi di licenza.

Gli asset di Wallpaper Engine e i wallpaper degli autori del Workshop restano
nelle librerie Steam dell'utente. Non sono inclusi nei sorgenti o nel runtime.
Il runtime locale di sviluppo contiene librerie di sistema estratte da pacchetti
Arch già presenti: non è un pacchetto portabile pronto alla distribuzione pubblica.

Correzioni coordinate monitor, ritaglio e zoom: `patches/engine-project-controls.patch`.

Compatibilita web e parser legacy: `patches/engine-web-compatibility.patch`.
Upload delle regioni modificate secondo il contratto OnPaint di CEF:
https://cef-builds.spotifycdn.com/docs/131.3/classCefRenderHandler.html

Riproduzione e controlli delle scene, lifecycle audio e output coperti:
`patches/engine-sound-controls.patch`.
Semantica del trasporto secondo la documentazione originale:
https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/ISoundLayer.html


Workshop integrato (0.12):
- **Steamworks / Valve**: ABI flat minima dichiarata in `src/steam_abi.hpp`,
  confrontata con gli header Valve esposti da Steamworks.NET (`CodeGen/steam`).
  ISteamUGC020 e dispatch manuale, con packing Linux a 4 byte. Documentazione:
  https://partner.steamgames.com/doc/api/ISteamUGC
  https://partner.steamgames.com/doc/api/steam_api
- Il redistributable proprietario `libsteam_api.so` non è incluso nei sorgenti.
  Per la demo locale viene riutilizzata la copia installata con Steamworks.js
  0.4.0 / WallpaperX. Qualunque distribuzione pubblica del runtime richiede una
  verifica dei termini Valve e delle licenze applicabili.
- **libcurl**, licenza curl: trasferimento delle anteprime statiche HTTPS da
  CDN Steam; libreria di sistema risolta tramite pkg-config.
- WallpaperX è stato consultato per l'uso delle API UGC (ricerca, tag,
  sottoscrizione e download). Nessun bundle JavaScript o eseguibile WallpaperX
  viene avviato dall'app. I download restano nelle librerie Steam dell'utente.

0.13: Pango/Cairo di sistema per shaping Unicode, bidi e fallback dei font;
rispettive licenze upstream (LGPL/MPL). Le categorie di transizione sono state
confrontate con la documentazione Adobe, senza usare codice o asset Adobe:
https://helpx.adobe.com/premiere/desktop/add-video-effects/effects-and-transitions-library/list-of-video-transitions.html
Contratto degli eventi multimediali:
https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/MediaTimelineEvent.html
https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/MediaPlaybackEvent.html

0.14: riferimenti cromatici, senza asset grafici o componenti UI importati:
- Catppuccin Latte/Mocha: https://catppuccin.com/palette/ (palette MIT).
- Dracula: https://draculatheme.com/contribute (palette MIT).
- Rosé Pine: https://rosepinetheme.com/palette/ (palette MIT).
Le varianti sono adattamenti per superfici, contrasto e accenti di Wallpaper Lab;
non costituiscono port ufficiali approvati dai progetti di riferimento.

0.15: temi indipendenti e integrazione opzionale Caelestia.
- Ispirazione visiva da Midnight e System24 (Discord), senza CSS o asset copiati:
  https://github.com/refact0r/midnight-discord
  https://github.com/refact0r/system24
- Comfy (Spicetify), per il rapporto tra superfici arrotondate e accento menta:
  https://github.com/Comfy-Themes/Spicetify
- Catppuccin Latte/Mocha e Rosé Pine/Dawn usano i rispettivi riferimenti cromatici
  già indicati sopra; gli altri preset sono palette ed effetti originali adattati
  alla UI. Nessun font o immagine dei temi viene scaricato o distribuito.
- Caelestia CLI installata separatamente, invocata solo per generare palette con
  `wallpaper --print`; il file scheme.json segue il contratto della shell installata:
  https://github.com/caelestia-dots/cli
  https://github.com/caelestia-dots/shell
- FFmpeg/ffprobe installati separatamente per metadata e miniature dei media
  locali, con le rispettive licenze dei pacchetti. I file personali importati non
  sono distribuiti con il progetto.


0.16: Material You è generato dall'extra Python **materialyoucolor** e Pillow,
installati separatamente solo su richiesta di attivazione. Algoritmi di riferimento
Material Color Utilities con licenza Apache-2.0; wrapper Python materialyoucolor
con licenza MIT. Pillow mantiene la propria licenza MIT-CMU e gli avvisi PIL. Non sono copiati nel repository o nel runtime base:
https://github.com/T-Dynamos/materialyoucolor-python
https://github.com/material-foundation/material-color-utilities
https://python-pillow.org/
Il helper e la tinta aggiuntiva delle superfici sono codice Wallpaper Lab.
Il formato pubblicato è compatibile con `Colours.qml` della shell Caelestia;
nessun file del pacchetto Caelestia viene sovrascritto.

Manipolazioni del fotogramma, durata/direzione/curva transizioni condivise con la
preview: `patches/engine-custom-animation-image-transform.patch`.

Transizioni per output e passaggio controller/renderer 0.17:
`patches/engine-transition-handoff.patch`. Separa gli snapshot nativi dai frame
dell'inspector, identifica richiesta e uscita, e sincronizza l'avvio tramite un
segnale monotono comune. I pin upstream restano invariati.

Composizione RGBA, iris antialias e namespace desktop dedicato 0.18:
`patches/engine-compositor-transitions.patch`, applicata dopo la patch handoff.
Pin invariati. La regola Hyprland è codice applicativo in desktop_surface.hpp;
i riferimenti visivi dei nuovi temi sono elencati in docs/THEMES.md.

- **Orbitron**, The Orbitron Project Authors; **Oxanium**, Severin Meyer;
  **Rajdhani**, Indian Type Foundry: SIL Open Font License 1.1. Originali non
  modificati scaricati da `google/fonts`, commit e hash nel manifest
  `assets/fonts/manifest.json`. Licenze complete, descrizioni e font sono inclusi
  in `assets/fonts/` e installati nella directory `fonts/` del runtime. Nessun
  font del gioco o dell'anime è stato estratto o ridistribuito.
- La correzione del mute live CEF/MPV è conservata in
  `patches/engine-live-audio-mute.patch`; i pin upstream sono invariati.

- **Silkscreen**, The Silkscreen Project Authors / Jason Kottke, SIL OFL 1.1:
  font pixel originale non modificato, incluso da `google/fonts` allo stesso
  commit fissato nel manifest. Licenza in `assets/fonts/silkscreen/OFL.txt`.
  Personaggi, scenari e decorazioni Pixel Theme sono disegnati nel codice dell'app.
