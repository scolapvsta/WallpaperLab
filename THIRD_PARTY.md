# Provenienza e licenze

Wallpaper Lab è un progetto derivato, distribuito con GPL-3.0. Il nome è provvisorio.

- **Almamu/linux-wallpaperengine**, GPL-3.0: renderer nativo, parser PKG/TEX,
  scene, video, web e integrazione desktop. Il commit è fissato nel submodule
  `vendor/engine`. Le modifiche locali sono in `patches/engine-integration.patch`.
- **danihek/hellpaper**, copyright 2025 danihek, e fork **Belial02/hellpaper**, MIT:
  base del selettore Raylib, shader, quattro layout e animazioni. `src/selector.c`
  è la versione modificata per questa applicazione; l'originale è fissato in
  `vendor/hellpaper`. Avviso MIT completo in `assets/LICENSE.hellpaper`.
- **raylib**, Ramon Santamaria e contributori, zlib/libpng: UI e immagini.
  Versione 5.5 fissata in `vendor/raylib`; licenza in `assets/LICENSE.raylib`.
  Il decoder GIF usa stb_image vendorizzato da Raylib, con i relativi avvisi nel sorgente.
- **Chromium Embedded Framework** e dipendenze del renderer mantengono le proprie
  licenze e attribuzioni upstream. Le risorse CEF sono scaricate dal build upstream;
  la distribuzione locale include i suoi avvisi di licenza.

Gli asset di Wallpaper Engine e i wallpaper degli autori del Workshop restano
nelle librerie Steam dell'utente. Non sono inclusi nei sorgenti o nel runtime.
Il runtime locale di sviluppo contiene librerie di sistema estratte da pacchetti
Arch già presenti: non è un pacchetto portabile pronto alla distribuzione pubblica.
