# Wallpaper Lab (nome provvisorio)

- Conversazione in italiano. Applicazione nuova e separata da caelestia-rice.
- Obiettivo: unire il selettore animato derivato da Hellpaper al renderer nativo
  di linux-wallpaperengine, in un'applicazione propria e personalizzabile.
- Un solo eseguibile applicativo: `wallpaper-lab`. La modalità `--render` usa
  direttamente la libreria del motore; non avvia gli eseguibili upstream.
- Conservare i quattro layout e la fluidità del selettore; distinguere anteprima
  Workshop da rendering della scena completa. Non promettere crossfade desktop
  o compatibilità con tutti i progetti se non implementati/verificati.
- Dipendenze upstream fissate nei submodule. Salvare le modifiche a engine/raylib
  nelle patch e aggiornare i pin solo dopo confronto e test. Non perdere modifiche
  locali quando si applicano patch o si aggiornano i submodule.
- Prima di fermare un renderer verificare PID, tempo di avvio ed eseguibile.
  Conservare il wallpaper precedente se il nuovo non parte.
- Backup, log, screenshot, build e dati della libreria personale restano in `local/`
  o nelle directory XDG della macchina. Non committare asset del Workshop.
- Fare test di catalogo, lifecycle dei processi e prove grafiche mirate; dichiarare
  limiti e risultati effettivi. Nessun cambio dell'avvio automatico senza backup.
- Repository privata: `scolapvsta/wallpaper-engine-customized`. Non cambiarne la visibilità.
- Il progetto Codex di riferimento è Wallpaper Engine Customized (WIP).
  Il nome definitivo dell'applicazione è ancora da decidere.
