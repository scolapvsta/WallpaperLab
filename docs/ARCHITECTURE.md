# Architettura

- `wallpaper-lab`: shell Raylib e controller C++ con API C. La modalità `--render`
  incorpora la libreria nativa del motore; non lancia renderer upstream.
- `catalog`, `project_source`, `project_properties`: catalogo Steam, risoluzione
  dei preset e proprietà persistenti per progetto.
- `workshop`: worker serializzato per Steam UGC, callback manuali, trasferimenti
  HTTPS limitati e interrompibili per le anteprime. Nessun codice remoto eseguito
  prima del download/importazione.
- `running.json`: stato con identità PID/start/exe. In modalità singola `outputs`
  associa ogni uscita a un processo indipendente; il controller sostituisce solo
  il processo del monitor selezionato. Un cambio globale sostituisce l'intero gruppo.
- `Transition.h`: postprocess OpenGL per output con snapshot del vecchio wallpaper
  e fotogramma corrente. Il controller conserva il renderer precedente fino alla
  readiness del nuovo; fallimenti di avvio non eliminano il precedente.
- `settings_ui`: menu disegnati alla fine del frame, con selezione differita e
  blocco degli eventi verso i campi coperti dall'overlay.
- `shaped_text`: cache limitata di layout Pango/Cairo; shaping Unicode e fallback
  Fontconfig. ASCII mantiene il renderer FreeType dell'interfaccia.
- `i18n`: 22 locale, fallback inglese per i messaggi avanzati delle nuove lingue.
- `patches` e `build-manifest.json`: modifiche vendor riproducibili con pin upstream.

Test e limiti effettivi sono raccolti in `tests/RESULTS.md`; build, log e dati
personali rimangono in `local/` o nelle directory XDG.
