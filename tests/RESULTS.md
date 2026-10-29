# Verifica iniziale — 26 ottobre 2026

- Compilazione Release del renderer dai sorgenti, 399 passi iniziali: riuscita.
  Corretto il rilevamento di GLFW nel CMake upstream per prefix non di sistema.
- Compilazione dell'app con Raylib statico e libreria del renderer: riuscita.
- Test del catalogo: deduplicazione delle directory, tipi case-insensitive,
  scena in PKG, video mancante, JSON malformato isolato, preset con dipendenza,
  rifiuto di un percorso di anteprima esterno al progetto. Superato.
- Test GIF: durata variabile dei frame, avanzamento, ritorno al primo frame e
  rilascio delle risorse. Superato.
- Libreria locale: 122 progetti rilevati; 80 scene, 20 video, 5 web e 17 preset.
- Ispezione visiva dei quattro layout e dell'anteprima GIF: effettuata con
  screenshot locali; ridimensionamento della finestra verificato tramite il
  compositor durante le prove. Titoli e tipo visualizzati nella griglia.
- Anteprima della scena Frieren: rendering reale riuscito, modalità fit con bordi
  neri corretti. La prima prova mostrava bordi stirati ed è stata corretta.
- Anteprima di un video Workshop con MPV: avvio e permanenza del renderer verificati.
  Un primo tentativo ha ricevuto una chiusura dal driver prima del controllo di
  permanenza; la prova successiva è passata. La causa della chiusura non è determinata.
- Migrazione da WallpaperX riuscita con backup locale. Un unico processo renderer
  della nuova app serve DP-6 (1920×1080) e DP-5 (2560×1440), modalità panoramica.
  Geometrie delle due superfici verificate attraverso `hyprctl layers -j`.
- Due applicazioni consecutive di Frieren: PID sostituito e precedente terminato.
- Scena con JSON volutamente non valido: applicazione rifiutata, PID del wallpaper
  precedente rimasto vivo e invariato.
- Stato PID riferito a un processo esterno: `--stop` non lo termina.
- Mappature del renderer installato: nessuna libreria caricata da WallpaperX o
  dai percorsi del build. Installazione indipendente dal vecchio runtime verificata.
- Aggiornamento del runtime con renderer attivo: installazione e ripristino automatico
  di Frieren verificati; conservata una versione precedente per rollback.
- Autostart scritto e comando di ripristino eseguito; non effettuato un logout/login.

Le prove grafiche esercitano le transizioni del selettore tramite modalità di test
interna e ispezione visiva, non un test completo di tutti gli input fisici.
Non sono stati provati tutti i wallpaper, i progetti web, audio, audio reattivo,
modalità duplicata/singolo monitor, tutti gli effetti o i casi fullscreen.
I preset con dipendenze sono esplicitamente non supportati in questa versione.
Non è implementato un crossfade delle scene desktop.

Gli avvisi GLFW relativi alla posizione delle finestre Wayland e quello GLEW
"No GLX display" compaiono nelle anteprime upstream; nei casi provati il rendering
è proseguito. Nessuno screenshot, log personale o dato della libreria è versionato.

## Versione 0.2 — finestra persistente, mouse e carousel (26 ottobre 2026)

- Build completa dai sorgenti eseguita nella cartella `Wallpaper Engine Customized (WIP)`.
  Corretto il quoting dei comandi Wayland scanner con una patch separata `VERBATIM`;
  le revisioni upstream rimangono invariate.
- CTest: catalogo, timing GIF e nuovi test preferenze/carousel superati. Verificati
  ordine persistente, riordino, rimozione, ciclo in entrambe le direzioni, wrap,
  esclusione di elementi mancanti/non supportati e lock fra processi.
- Configurazione della scorciatoia: test con compositor simulato superati per
  conflitti, backup, include idempotente, disattivazione e rollback se il binding
  non viene registrato. Verificato anche l'avvio effettivo del binario nei test.
- Installazione del runtime aggiornato e ripristino di Frieren riusciti.
- Test di integrazione reali superati: catalogo 122 elementi, anteprime native
  scena/video, sostituzione, scena non valida che conserva il precedente,
  rifiuto di terminare un processo estraneo.
- Carousel desktop reale: vuoto e controller occupato conservano il PID; ciclo
  scena/video, ritorno al primo, precedente e salto di un elemento mancante riusciti.
  La configurazione e la scelta precedenti sono state ripristinate al termine.
- Win/Super+Tab installato su Hyprland 0.56.2 Lua; assenza di conflitti verificata.
  Una pressione simulata tramite dispositivo di input ha applicato il successivo
  wallpaper del carousel. Nessun errore di configurazione Hyprland dopo il reload.
  Autostart invariato; backup della configurazione Hyprland nello stato XDG dell'app.
- Mouse: aggiunta dello sfondo attuale e selezione/aggiunta di un secondo elemento
  verificate con input reale. La sequenza automatizzata completa via coordinate
  non è una prova superata: cambi di geometria/focus l'hanno interrotta. I test
  di ciclo e shortcut sopra non dipendono da quella sequenza.
- Finestra persistente: la modalità di test ha applicato realmente Frieren e
  continuato per 646 frame, navigando poi tutte le schermate senza uscita
  automatica. La chiusura finale ha lasciato il renderer attivo.
- Font di sistema, quattro layout, carousel popolato, schede monitor, riproduzione,
  scorciatoia e anteprima Workshop ispezionati tramite screenshot locali.

Limiti: questa è una prima UI utilizzabile col mouse, non la revisione grafica
finale. La gestione nativa delle scorciatoie è stata provata su Hyprland Lua;
la generazione Hyprlang è presente ma non provata su un compositor precedente.
Non è un'app Windows. Modalità duplicata e singolo monitor, audio/interattività e
progetti web non hanno ricevuto una nuova prova di rendering completa. Nessun
crossfade desktop, rotazione temporizzata, assegnazione diversa per ogni monitor
o download Workshop. La lista di preferiti dell'utente non è stata popolata con
scelte arbitrarie; gli elementi delle prove sono stati rimossi.

## Versione 0.3 — popup carousel (26 ottobre 2026)

- Libreria e gestione dei preferiti sempre a griglia. La scelta Griglia/Fiume/
  Verticale/Onda viene salvata separatamente e usata dal popup.
- Build nativa e quattro test CTest superati: catalogo, GIF, preferenze e popup.
  Il nuovo test verifica eventi fuori ordine, passi assoluti, wrap, lista vuota
  o singola, rilascio anticipato, sessioni concluse, istanza unica e riapertura.
- Configurazione shortcut con compositor simulato: conflitti, backup,
  persistenza, disattivazione e rollback superati, inclusi i binding di rilascio.
- Prova su Hyprland 0.56.2 Lua con input da tastiera virtuale: cinque Tab nella
  stessa pressione di Windows mantengono una sola finestra e lo stesso renderer;
  il rilascio chiude il popup e applica il quinto elemento atteso, con wrap.
  Nell'esecuzione registrata la finestra è stata rilevata dopo circa 350 ms
  dall'inizio della sequenza e scomparsa entro 67 ms dal rilevamento del rilascio.
  Sono misure del test locale, non garanzie di latenza.
- Esc annulla senza cambiare il PID del renderer. Chiusura manuale del popup
  annulla; eventuali cambi di focus non confermano o chiudono la selezione.
- Quattro layout ispezionati tramite screenshot locali: miniature, evidenziazione,
  titolo e contatore della scelta. Verificato il caso di rilascio ricevuto prima
  dell'apertura: nessuna finestra residua, applicata la selezione finale.
- Il test live conserva byte per byte la lista dei preferiti e ripristina le
  impostazioni e lo sfondo precedenti. Runtime aggiornato installato localmente.

Il popup globale richiede attualmente Hyprland Lua; la vecchia configurazione
Hyprlang non implementa la sequenza pressione/rilascio. Su altri compositor
resta disponibile il comando diretto `--next`. Nessun nuovo intervento su
autostart, renderer upstream o download Workshop. Gli input sintetici sono
stati provati nella sessione locale: non coprono ogni tastiera, combinazione
personalizzata o applicazione che inibisce le scorciatoie globali.

## Versione 0.3.1 — popup compatto e trasparente (26 ottobre 2026)

- Eliminati intestazione, footer, margini e cornice del popup. Immagini opache,
  selezione con bordo interno; framebuffer trasparente negli spazi non occupati.
- Verifica grafica sul desktop con quattro preferiti: griglia 320×320, fiume
  496×160, verticale 160×496, onda 544×208. Le due strisce sono riempite dalle
  immagini; nell'onda si vede il desktop fra le immagini, senza ombra o blur
  rettangolare. La regola locale di opacità generale non sbiadisce le miniature.
- Con configurazione XDG temporanea, provati carousel vuoto, singolo elemento,
  tre elementi e nove elementi: rispettivamente 320×80, 160×160, 320×320 e
  480×480. Griglia 3×3 e ultima cella trasparente nel caso di tre elementi
  ispezionate visivamente. Nessuna modifica ai preferiti reali per queste prove.
- CTest 4/4 superati: aggiunti limiti geometrici fino a 2048 elementi e monitor
  piccoli, lettura selettiva dei progetti con duplicati/mancanti e regressione
  della configurazione del popup. Verificato anche il rollback shortcut col
  compositor simulato. Nessun errore di configurazione Hyprland dopo il reload.
- Test live completo superato nuovamente: cinque Tab, istanza singola, renderer
  invariato durante la selezione, conferma, Esc, quattro modalità e rilascio
  anticipato. Preferiti conservati e wallpaper precedente ripristinato.
- Rimossa l'animazione di ingresso del compositor e la crescita delle immagini
  all'apertura; letti solo i progetti preferiti, decodifica parallela alla
  creazione della finestra, nessun caricamento del font principale. Scorrimento
  con interpolazione breve e stabile rispetto alla frequenza dei frame.

La misura dalla partenza della sequenza sintetica alla finestra rilevata resta
circa 350 ms, simile alla versione precedente; include i ritardi programmati
dell'input. Non dimostra un miglioramento del tempo di avvio del processo e non
significa apertura istantanea. La nuova presentazione evita invece animazioni
di ingresso e attese aggiuntive prima di mostrare le immagini. Rendering delle
scene e relativi tempi di caricamento invariati. Trasparenza provata su questa
sessione Hyprland, non su tutti i compositor.

## Versione 0.3.2 — contorno, zoom e dissolvenze (26 ottobre 2026)

- Griglia, fiume e verticale hanno un fondo scuro con contorno sottile. L'onda
  conserva lo sfondo trasparente senza cornice. Dimensioni dei popup invariate.
- Eliminato il contorno sulla selezione. L'immagine scelta cresce, la precedente
  si riduce e il cambio accompagna lo scorrimento; la scelta viene disegnata
  davanti alle altre. Verificate miniature sui bordi e cambi in tutti i layout.
- Dissolvenza dell'intero popup, immagini e contorno insieme: 140 ms con curva
  simmetrica in entrata/uscita. Un rilascio durante l'apertura inverte la curva
  dalla posizione raggiunta. La scelta si fissa al rilascio; Invio nel popup
  non conferma più una scelta, mentre Esc e chiusura della finestra annullano.
- CTest 4/4 superati, inclusi test di simmetria a 30/60/120 FPS, inversione
  durante il fade-in e zoom senza salti o overshoot. Test shortcut simulato e
  controllo errori di configurazione Hyprland superati.
- Prova grafica con XDG isolato superata: screenshot di ingresso, selezione
  iniziale, cambio, selezione finale e uscita per tutte le modalità. Verificata
  la riapertura di una nuova sessione ricevuta durante il fade-out, senza perdita
  della richiesta. Il test non ha avviato renderer né modificato dati personali.
- Nella prova con tastiera virtuale sono passati cinque Tab, istanza unica,
  renderer invariato durante la selezione, scelta finale ed Esc. La scomparsa
  della finestra dopo il rilascio è stata rilevata dopo circa 160 ms.
  La sequenza desktop completa non viene dichiarata superata: le ripetizioni
  hanno incontrato conteggi di input o cambi di PID inattesi nella sessione
  condivisa. La causa non è stata isolata; il controllo grafico e del passaggio
  fra sessioni è stato completato separatamente con stato XDG temporaneo.

Runtime installato localmente. Il tempo di avvio del contesto grafico e quello
di caricamento della scena desktop restano distinti dall'animazione del popup.

## Versione 0.3.3 — immagini unite e bordi morbidi (26 ottobre 2026)

- Bordo bianco sottile su tutte le immagini, comprese quelle dell'onda. Maschera
  con antialias per gli angoli arrotondati delle finestre e delle immagini in
  primo piano; nessun contorno esterno rettangolare nell'onda.
- Griglia composta da celle piene, con ingrandimento sovrapposto della selezione:
  nessuna fessura quando l'immagine precedente si riduce. Fiume e verticale usano
  segmenti contigui di dimensione variabile, mantenendo gli estremi delle immagini
  adiacenti coincidenti durante lo scorrimento e lo zoom.
- Ingrandimento nominale del 28% per griglia e onda; espansione del 38% lungo
  la striscia orizzontale/verticale. Proporzioni preservate con ritaglio centrale,
  miniature decodificate a 256×256 per sostenere la maggiore dimensione.
- CTest 4/4 e controllo del diff superati. Prova grafica con stato XDG isolato
  superata nei quattro layout: ispezionati ingresso, cambio di selezione,
  risultato assestato e uscita. Verificata anche una nuova sessione durante
  la chiusura; nessun renderer avviato dal test, nessun input di tastiera iniettato.
- Con quattro preferiti: griglia 320×320, fiume 496×160, verticale 160×496,
  onda 544×253. L'onda ha spazio verticale aggiuntivo per non tagliare la scelta
  ingrandita. Nessun errore di compilazione degli shader nei log della prova.

La dissolvenza simmetrica di 140 ms e il comportamento pressione/rilascio
rimangono quelli della versione precedente. Runtime aggiornato installato
localmente; non sono state modificate le liste personali dei preferiti.

## Versione 0.3.4 — zoom oltre il contorno e movimento graduale (26 ottobre 2026)

- Bordi bianchi portati da 1,2 a 2,6 pixel logici. Il pannello decorativo viene
  disegnato separatamente dalle miniature ingrandite: la selezione non è più
  limitata dalla maschera arrotondata del gruppo, anche negli angoli della griglia.
- Superficie nativa con 36 pixel trasparenti per lato per ospitare lo zoom.
  Il pannello visibile conserva le dimensioni precedenti; con quattro elementi
  le superfici complete sono 392×392, 568×232, 232×568 e 616×325. Sui monitor
  piccoli lo spazio viene dimensionato rispettando il limite disponibile.
- Fiume e verticale: passo fisso, miniature quadrate uguali; solo la selezione
  cresce del 38% in entrambe le direzioni e sporge dal pannello. La precedente
  torna gradualmente alla dimensione normale. Griglia e onda mantengono lo zoom
  del 28%, con disegno della scelta in primo piano.
- Scorrimento e zoom usano un movimento smorzato che conserva la velocità fra
  pressioni successive: circa 350 ms per coprire il 95% di un singolo passaggio.
  Il contatore di avanzamento resta continuo anche nel wrap, evitando che il
  percorso più corto provochi inversioni durante una sequenza rapida di Tab.
- CTest 4/4 superati: verificati spazio per lo zoom, limiti del monitor,
  avanzamento monotono con richieste ripetute a 30/60/120 FPS e assenza di
  overshoot. La dissolvenza di apertura/chiusura resta simmetrica a 140 ms.
- Prova con stato XDG isolato superata su tutti i layout, inclusi più passi
  consecutivi e una nuova sessione durante la chiusura. Il desktop era bloccato:
  gli screenshot del compositor mostravano il blocco e non sono stati usati
  come prova visiva dell'app. Il controllo è stato effettuato sui canvas RGBA
  esportati dall'app stessa, prima del fade, con l'opzione esplicita di test.
  Ispezionati fotogrammi intermedi e finali; verificati pixel trasparenti nel
  margine e pixel della selezione oltre i bordi di griglia e strisce.

Nessuna interazione con la schermata di blocco, nessun input sintetico e nessun
renderer avviato dalle prove grafiche. Runtime aggiornato installato localmente.

## Versione 0.3.5 — wallpaper audio responsive (26 ottobre 2026)

- Riprodotto dal log il crash `close symbol missing`: CEF veniva caricato dopo
  libc. Link diretto di CEF nell'eseguibile, prima di libc, senza modificare i pin.
- Separati “Suono dello sfondo” e “Reazione alla musica”. Anche le impostazioni
  precedenti con `muted: true` mantengono attiva la cattura per i progetti audio.
- Cattura stereo float dal monitor dell'uscita PulseAudio/PipeWire. Corrette
  gestione dei frammenti, latenza del main loop, valori negativi dello spettro
  e ritorno al silenzio. Aggiunta sottoscrizione ai cambi dell'uscita predefinita.
- Collegati `wallpaperRegisterAudioListener` nei web (128 valori) e
  `engine.registerAudioBuffers` nelle scene (Float32Array left/right/average,
  16/32/64 bande). Proprietà iniziali e FPS inviati ai progetti web; mute CEF
  separato dall'analisi audio. Gli shader ricevono canali distinti.
- Corretti destinazione texture di OnPaint, gestione dei riferimenti CEF e
  ordine di chiusura. Il segnale di avvio web attende caricamento e primo paint.
- Durante la prova con musica, isolato tramite core un crash nel cambio
  copertina: glGetnTexImage non era disponibile nel contesto OpenGL richiesto.
  Sostituito con la funzione core glGetTexImage, con buffer RGBA dimensionato.
- CTest 4/4, compilazione Release, controllo diff e verifica inversa delle tre
  patch del motore superati. Test scorciatoie con compositor simulato superato.
- `tests/audio_integration.py`: passati separazione dei controlli nel piano di
  avvio, cattura disabilitata senza stream, proprietà web, reload del documento,
  HTML mancante senza readiness e avvio fallito con processo precedente preservato.
  Testati toni sinusoidali a 440 Hz su un'uscita virtuale: sinistra/destra separati,
  canale inattivo a zero, ritorno al silenzio. Fixture web, “Audio Visualizer”
  893418273 e fixture SceneScript con tre risoluzioni e array persistenti passati.
  Tutti con volume wallpaper zero e 30 FPS. Shutdown normale con codice 0.
  I tempi osservati dal polling del test sono inferiori a 0,3 s; non rappresentano
  una misura precisa della latenza audio. Nessun tono inviato alle cuffie.
- Ispezionati i canvas esportati dal visualizer reale durante il tono; gli
  elementi grafici reagiscono ai dati ricevuti. Avvio del runtime installato
  sul desktop duplicato DP-6/DP-5 verificato per quattro secondi; ripristinata
  poi la scena presente prima della prova. I cambi successivi dell'utente restano
  indipendenti dalla prova.
- Regressione sul runtime installato: catalogo 122 progetti, anteprima scena e
  video e preservazione dei processi estranei passati. Runtime precedente
  conservato dall'installazione, nessuna modifica all'avvio automatico.

Evidenze locali: `local/audio-integration-final/results.json`, canvas e log nella
stessa directory, `local/audio-desktop-results.json`, `local/logs/audio-desktop.log`
e `local/audio-native-regression.json`. Patch in `patches/engine-audio-web.patch`.
Non verificati tutti i wallpaper della libreria, né il cambio fisico delle cuffie
in questa sessione. Alcuni operatori audio delle particelle e altre API avanzate
upstream restano incompleti; la risposta in frequenza non replica esattamente
Wallpaper Engine per Windows. Non è una garanzia di compatibilità universale.


## 2026-10-28 — UI refresh 0.4.0

- Build Release completed; CTest: catalog, GIF timing, preferences, popup all passed.
- Popup focus regression: 3,000 phases each for 1, 2, 4 and 21 entries;
  focus stays bounded, sums to one and is invariant across cyclic wraps.
- Live popup test with isolated XDG state, four personal preview references only:
  all four layouts passed, 32 rapid additional advances per layout, fixed native
  surface sizes, wrap, cancellation, and a new session during fade-out. No desktop
  renderer was started. Captures inspected for grid, vertical and wave transitions.
- Native application canvas captures inspected: dark/light library, display
  monitor selection, playback controls, appearance and confirmation dialog.
- Disposable project trash test passed on the home filesystem: files and
  `.trashinfo` recovery metadata retained, invalid project/symlink rejected.
  `/tmp` is a separate internal mount here and GIO refuses trashing from it;
  the test fixture now lives on the build filesystem. No permanent-delete fallback.
- Mouse integration script added (`tests/ui_refresh_integration.py`), but real
  clicks could not reach the app because the desktop was locked. Not counted as
  passing; real mouse interaction and the original recording still need user review.
- Live scene action continues to use a separate renderer window, not embedded
  scene rendering. Individual monitor selection replaces the active renderer group;
  independent simultaneous wallpapers per monitor are not implemented.
- Evidence is local under `local/ui-refresh/`; no Workshop assets committed.
- Installed runtime 0.4.0 with previous runtime backup. Frieren renderer restored
  successfully (owned PID/start identity verified through `--status`). Personal
  settings and carousel JSON contents compare equal to the pre-install backup.
  Library launched for review; no autostart or shortcut configuration changed.


## 2026-10-28 — UI redesign 0.5.0

- Native shell separated into `src/library_ui.h`; one shared set of text,
  button and vector icon components for Library, Carousel, Settings and dialogs.
- Larger grid, centered editable search, combined type/resolution/audio filters,
  animated inspector/reflow, pinned actions, white empty/filled stars, trash on
  every tile, moon theme toggle and display gear. Own SVG desktop identity.
- Application canvas captures inspected on a compositor-tiled 1263×673 surface,
  including dark/light library, selected inspector, filters, all settings tabs
  and trash dialog. The primary actions fit even below the requested minimum height.
- Adwaita body, DejaVu Sans Bold controls and provisional Noto Serif wordmark;
  CJK title glyphs rasterized from the installed TTC with FreeType. Actual Chinese
  titles visually checked without missing glyphs. No system fonts vendored.
- Catalog tests cover PKG JSON metadata, combined filters, exact FHD/QHD,
  ultrawide/portrait, unknown metadata, and truncated packages. Existing catalog,
  preferences/trash, GIF timing and popup tests all pass.
- Real video dimensions observed in library captures through optional ffprobe;
  scene dimensions read from orthogonal projection. Audio filter uses declared
  `supportsaudioprocessing`; no metadata is inferred from thumbnail sizes.
- All four popup layouts passed native rapid-repeat/cancel/wrap/session-handoff
  checks with isolated XDG state. Wave left/right caps visually checked at both
  moving and settled phases. No renderer started by these popup tests.
- Shortcut UI records actual modifiers and key, validates/conflict-checks through
  the existing backend and activates the result. Only this app's own current
  binding is suspended in memory during capture, restored on cancel/normal close.
- Mouse integration script updated with sidebar/filter/persistence/trash checks
  and opt-in atomic UI state evidence. Execution remains blocked by the visible
  desktop lock surface; NOT reported as passing. Recorded-key interaction also
  still requires an unlocked desktop for end-to-end verification.
- All screenshots/logs remain under `local/ui-redesign/`. Workshop assets and
  personal files were not removed or committed; no submodule pins changed.
- Installed 0.5.0; staged executable validated before replacement. Previous
  runtime retained. Original wallpaper alive after restore, settings/carousel/
  shortcut JSON contents unchanged. Installed canvas smoke completed with
  dark/light inspector actions and all settings tabs inspected. Previous GUI
  closed through the compositor after checking executable, PID and start time;
  new personal library launched for review. Autostart unchanged.


## 2026-10-28 — Physical display calibration 0.6.0

- Added display parsing, real panel-size canvas, logical-pixel fallback, drag
  snapping, top/center/bottom alignments, named connector persistence, overlap
  validation and immutable renderer snapshots. OS monitor/window/cursor geometry
  is not changed. Preferences are backed up before saving a layout.
- Five CTest targets pass, including the new display-layout target: mixed DPI,
  top/bottom/center alignment, negative offsets, seam rounding, connector order,
  profile round trip, rotation, scale, missing physical size and invalid payloads.
- Native output regression uses a static HTML gradient, isolated XDG state and
  explicit opt-in framebuffer capture. Across 400 matched physical heights, mean
  absolute red-channel difference dropped from 18.92 (pixel layout) to 0.18
  (physical layout). Both native output sizes remained 1920×1080 / 2560×1440.
  The personal Cyberpunk renderer retained its original PID/start identity
  throughout this isolated test. Evidence: local/display-layout-tests/web-graphics/.
- The earlier FFV1 video probe was interrupted by the PC/session crash and is
  NOT a passed result. Its log reports hardware decoding. The static web probe
  avoids that decoding path; no claim about the cause of the crash is made.
- App canvas captures inspected: two correctly proportioned, top-aligned panels,
  native resolutions and measured cm sizes; basis switch, alignment controls,
  save/cancel and always-visible footer action. Snapshot capture also works
  behind the desktop lock surface.
- Mouse drag integration script added, but the visible desktop lock intercepted
  clicks. It is NOT counted as passing; actual pointer drag needs unlocked review.
- Engine changes preserved in patches/engine-display-layout.patch, after the
  pre-existing integration/build/audio patches. Clean application over copies of
  the preserved local baseline reproduces all three files byte-for-byte; reverse
  check on the working engine passes. Dependency pins were not changed.
- Installed 0.6.0 with previous runtime retained. Saved the measured physical
  top-aligned profile after backing up preferences, then restored Cyberpunk 2077
  City Loop. PID/start/executable and --lab-layout snapshot verified. Native
  monitor geometry, other preferences, carousel and shortcut remained unchanged.
  Display settings opened directly for user review; autostart unchanged.

## 2026-10-28 — Placement and switcher controls 0.7.0

- Five CTest targets pass. Added per-project position persistence, independent
  projects, removal/readdition of favorites, center reset, invalid/NaN input,
  lock conflicts and corrupt-profile preservation checks. Catalog checks cover
  exact dual-display families, dynamic dimensions/counts and standard deduplication.
- Fresh-process CLI save/read and `--plan` propagation passed in disposable XDG
  directories. Centered profiles omit the renderer override.
- `tests/placement_graphics_integration.py`: static HTML gradient rendered through
  the native composition path on both displays. Right/down translation mean
  color errors 0.056 / 0.044 on a 0–255 scale, black padding verified, native sizes
  retained and the personal renderer unchanged. No video probe used.
- `tests/switcher_preview_integration.py`: all four actual popup layouts advanced
  through five captured steps and closed automatically. No renderer registry or
  shortcut session created; personal wallpaper unchanged.
- Isolated `--ui-test` passed; inspected captures for the placement inspector,
  Carousel controls and scrollable resolution dialog in dark/light themes.
  Evidence remains under `local/placement-tests/`.
- Mouse integration attempted once and blocked by the Caelestia lock screen;
  no successful pointer/drag interaction is claimed. No unlocking attempted.
- Placement engine patch passed reverse check on the current source and clean
  application against the preserved baseline. Upstream submodule pins unchanged.
- Installed 0.7.0 with the previous runtime retained. Verified restored Frieren
  process identity (PID/start/executable), unchanged personal configuration and
  unchanged physical span calibration. No personal placement was modified by
  tests. Opened the updated library for review.

## 2026-10-28 — Libreria e carousel 0.8.0

- Build completa con scripts/build.sh e cinque target CTest superati. Test
  aggiunti per risoluzioni multiple dichiarate (AND), dimensioni con × Unicode,
  esclusione dei formati insoliti dalla lista comune e griglia adattiva con
  miniature di almeno 210 px prima del cambio di colonne.
- Filtri comuni raggruppati secondo la documentazione ufficiale Wallpaper Engine.
  Le varianti selezionabili sono proprieta combo di risoluzione dichiarate dal
  progetto. Verificati CLI, persistenza, coesistenza con la posizione e passaggio
  dell'opzione al renderer. Non vengono inventate varianti per i progetti privi
  di questi metadati; nei file personali attuali non risultano combo dedicate.
- Prova grafica statica web: vista live ridotta, offset aggiornato sullo stesso
  PID, ricentraggio dal profilo persistente e arresto del trasferimento quando
  la richiesta della scheda non esiste. Wallpaper personale preservato.
  Evidenza: local/ui08-tests/live-final/.
- Prova libreria con scena web isolata: selezione applicata, finestra rimasta
  aperta, catture della vera scena nel pannello laterale; scena non valida
  respinta conservando il renderer precedente. Evidenza: library-final/.
- Quattro layout carousel: pressioni rapide, wrap, annullamento e nuova sessione
  durante la chiusura superati. Con tre preferiti, Fiume e Verticale hanno ora
  480 px di spazio per tre immagini da 160 px. Ispezionate catture della griglia
  con segnaposto e bordi arrotondati durante lo zoom. Anteprima autonoma dei
  quattro layout superata senza applicazioni desktop. Evidenza: popup/ e
  switcher-final/ sotto local/ui08-tests/.
- Dopo lo sblocco della sessione, prova con input del mouse tramite dispositivo virtuale superata:
  stella, clic con applicazione automatica, trascinamento della posizione senza
  riavvio/applica, ricentraggio persistente, filtri AND positivi/negativi,
  ridimensionamento del pannello, seconda selezione, conferma e cestino
  recuperabile. Solo fixture web e configurazione isolata, renderer personale
  invariato. Evidenza: local/ui08-tests/mouse-verified/.
- Il test di trascinamento usa movimento relativo del dispositivo virtuale:
  il warp del cursore Hyprland durante una presa non invia gli stessi eventi
  di movimento alla finestra. Il controllo acquisisce anche la posizione finale
  al rilascio, evitando di perdere un movimento rapido tra due fotogrammi.
- Tutte le sei patch motore applicate a una copia dei file del commit fissato:
  riproducono esattamente i file modificati. Validatore della serie verificato
  da sorgenti puliti, stato parziale, serie gia applicata e conflitto locale.
  Nessun pin upstream modificato.
- Installata 0.8.0 con runtime precedente conservato. Verificati PID, tempo di
  avvio ed eseguibile del renderer ripristinato (Cyberpunk 2077 City Loop),
  configurazione personale e calibrazione monitor identiche al backup.
  Libreria aggiornata riaperta per la revisione.

## 2026-10-28 — Workshop 3257420414, lente X-Ray

- Riprodotto il crash originale: padding testo `"32.00000 32.00000"` letto
  come intero in un helper `noexcept`. Il renderer ora accetta il vettore e gli
  errori di tipo possono raggiungere il normale confine di errore.
- Corretto lo shader Workshop di clipping: restringimento esplicito delle
  coordinate nelle espressioni UV vec2, senza cambiare gli asset scaricati.
- Completato il percorso delle proprieta SceneScript: namespace QuickJS,
  inizializzazione dopo la creazione dei livelli, callback delle proprieta,
  scriptProperties, modifica dei livelli, lettura della posizione del cursore,
  callback del mouse sui rettangoli trasformati e visibilita degli effetti.
  Corretti anche il trasferimento delle componenti z/w e i gradi di SceneScript
  rispetto ai radianti del renderer.
- I compose layer nascosti referenziati dalle dipendenze ora producono la
  texture necessaria alla lente, con alpha, anche senza effetti propri.
- `ctest`: 7/7 passati, incluso il nuovo test di padding, recupero degli errori
  JSON e vera compilazione GLSL dell'espressione UV corretta.
- Fixture SceneScript senza asset Workshop: init/applyUserProperties,
  scriptProperties, setter visibile/vettori/angoli, visibilita dell'effetto,
  cursorMove con coordinate world, arresto pulito. Evidenze in
  `local/xray-debug/script-native-final/`.
- Regressione audio: web, stereo, cattura disattivata, SceneScript e arresto
  pulito passati; `local/xray-debug/audio-regression/results.json`.
- Progetto reale: avvio e rendering su DP-6 e panorama DP-6 + DP-5 con
  calibrazione fisica 4931x1440. Framebuffer acquisiti con due posizioni del
  cursore; la lente rivela il secondo video e si sposta fra le superfici.
  Nessun errore di shader, setup oggetti o SceneScript nella prova finale;
  arresto pulito e renderer personale preservato. Evidenze:
  `local/xray-debug/real-panorama/`.
- Durante il gioco fullscreen la prima prova panoramica non completava il
  segnale ready; ripetuta con il gioco ridotto a icona, con esito positivo.
  Non sono state modificate finestre o impostazioni del gioco dal test.
- Limiti: l'esempio predefinito dell'autore e 16:9 nel canvas 5120x1440, quindi
  il panorama mostra margini neri. Caricamento di materiali personali dalla UI,
  payload completo degli eventi, picking sui pixel alpha e API SceneScript
  avanzate non sono coperti. Nessuna promessa di equivalenza per tutte le scene.

- Runtime aggiornato e progetto `3257420414` applicato tramite il controller
  installato: exit 0, stato `alive: true`. Mouse abilitato con backup delle
  impostazioni; runtime precedente conservato dall’installer. Interfaccia
  riaperta. Nessuna modifica all’autostart o ai file Workshop.

## 2026-10-28 — proprieta creator e modalita monitor

- Corretto WaylandMouseInput: `position` del viewport non veniva valorizzata;
  singolo/duplicato ora usano `globalPosition`, fornita da xdg-output.
- Argomenti di scaling associati esplicitamente al relativo output.
- Catalogo e pannello leggono lo schema del creator; override validati, salvati
  per progetto con backup e trasmessi tramite `--set-property`. Conferma slider
  al rilascio, applicazione tramite il controller esistente.
- Aggiunti zoom e pan limitato al ritaglio. La trasformazione del mouse segue
  zoom/posizione; mantenuta la calibrazione fisica dei panorami.
- CTest: 8/8 passati. Nuovo test di condizioni, combo, validazione numerica,
  etichette HTML inerti e limiti UV. Persistenza/argomenti verificati con XDG
  isolato; valore combo invalido rifiutato.
- Fixture SceneScript sul monitor DP-5, con origine non nulla: world X da 192 a
  448, uguale alla posizione del livello; arresto pulito e renderer personale
  preservato (`local/project-controls/single-mouse/`).
- Progetto reale 3257420414: rendering singolo DP-5 e duplicato DP-5/DP-6;
  prova maschera arrotondata tramite override x=false/newproperty6=true.
  Screenshot framebuffer, nessun errore shader/script e arresto pulito in
  `local/project-controls/real-single/` e `real-circle/`. La resa delle maschere
  non e dichiarata identica pixel per pixel a Windows.
- Probe grafico zoom 200%: errore medio pan orizzontale 0.008 livelli colore,
  verticale 0.030; nessun padding nero, dimensioni native preservate.
  `local/project-controls/crop-graphics/results.json`.
- Screenshot UI con libreria/configurazione isolate: `ui-shots/view-1.png`.
- Limiti: condizioni composte, file/directory/scenetexture, traduzioni complete
  delle etichette Wallpaper Engine, scelta colore grafica, payload eventi e
  impostazioni avanzate di qualita del motore non sono completati.

- Runtime installato, interfaccia riaperta e wallpaper 3257420414 applicato
  tramite il controller: exit 0, renderer alive. Backup runtime conservato.

## 2026-10-28 — Personalizzazione UI 0.9.0

- Build UI separata in build-ui09/ per evitare sovrascritture dell'eseguibile
  durante il lavoro parallelo sul renderer. Le modifiche del motore e delle
  proprieta creator presenti nella cartella sono state conservate.
- Otto target CTest passati, compreso il nuovo i18n: tutti i 209 messaggi del
  catalogo verificati nelle dieci lingue, fallback dei dati non tradotti,
  cambi di lingua e stabilita dei puntatori. Generazione con verifica dei
  parametri printf e delle celle mancanti.
- Prova con input reale del mouse e preferenze isolate superata: icone e
  disposizione header, dieci lingue cambiate nella stessa finestra, undici
  temi, tre densita, font cambiato senza riavvio e famiglia TTC FreeType.
- Shortcut registrato nel Carousel senza aprire impostazioni, evidenziato in
  verde; nuova combinazione verificata anche nella scheda Carousel delle
  impostazioni. Persistenza e verifica binding eseguite contro un backend
  Hyprland simulato: i binding personali non vengono riscritti dal test.
- Aggiunto un progetto mentre la finestra era aperta, con titolo che cambia
  l'ordine della griglia. Refresh tramite icona: selezione corretta per percorso,
  preferiti identici e stesso PID/start del renderer. Texture e worker ricaricati.
- Catture ispezionate per interfaccia tedesca/inglese, Glass/Pastello, registrazione
  inline e libreria dopo refresh. Evidenza: local/ui09-tests/verified/.
- Il test usa solo fixture web statiche per la parte desktop; nessuna ulteriore
  decodifica video viene avviata per verificare i temi o i font.
- Verifica finale del refresh dopo i reset dei contesti indicizzati: superata,
  con renderer, selezione e preferiti conservati (refresh-final/).
- Quattro popup demo verificati con tema RGB, lingua inglese e font TTC:
  avanzamento automatico, chiusura, nessun renderer avviato dal test e renderer
  personale invariato (local/ui09-tests/popup-verified/).
- Installata 0.9.0 dalla build privata, runtime precedente conservato. Verificati
  eseguibile/PID/tempo di avvio e stesso wallpaper; configurazione personale,
  proprieta per progetto, preferiti, shortcut e calibrazione identici al backup.
  Aperte le impostazioni Aspetto per la revisione.

## 2026-10-28 — Prestazioni, compatibilita e UI 0.10.0

- Baseline e log privati: `local/ui10-baseline/`; audit in `local/ui10-audit/`,
  correzioni in `local/ui10-audit-fixed/`, verifica finale web in `local/ui10-web-final/`.
- Audit sequenziale dei 102 progetti installati in single, mirror e span: 306
  avvii reali sui monitor DP-5/DP-6. La prima passata ha rilevato 10 progetti
  problematici. Ripetuti i 9 progetti corretti (27/27) e tutti gli 11 web dopo il
  cambio a URL file (33/33). Il risultato combinato e 303/306: 101 progetti
  avviati nelle tre modalita. Ciascun caso verifica readiness e sopravvivenza di
  0,6 s dopo readiness, a 30 FPS; non certifica tutti gli effetti o interazioni.
  I processi di prova hanno XDG isolato; PID, start ed eseguibile controllati prima
  dello stop. Il renderer personale e rimasto identico durante le prove.
- Unico progetto ancora non supportato: `3477054430`, Cat with headphones on the
  roof. Ha `orthogonalprojection: null`, camera prospettica e modelli `.mdl`.
  Non e una risoluzione 2D mancante: serve implementare rendering 3D, illuminazione
  e materiali. Catalogo e renderer ora comunicano una motivazione specifica.
- I sette preset dipendono da Audio Visualizer `893418273`. Le dipendenze vengono
  risolte localmente, con schema/valori ereditati, cache di risorse e identita
  separata per preferiti e profili. I file Steam non sono stati modificati.
  Una prova extra ha rilevato che la sola readiness non dimostrava il caricamento
  dello sfondo del preset: risolto usando la pagina fisica file://. Confermata
  l'immagine reale 2560x1440 di B&W Minimalist tramite naturalWidth/naturalHeight.
- Ayanami Rei Evangelion `2096554036`: stack da core con eu-stack ha localizzato
  il crash nel costruttore CParticle. Il campo `particle: null` sul Clock veniva
  selezionato prima di `text`. Corretto il dispatch e aggiunto test regressione.
  Perfect Wallpaper `884307090`: textinput senza valore ora vuoto; le stringhe
  textinput non includono piu virgolette JSON aggiuntive.
- Audio Visualizer a 4480x1440: 60 FPS nel test isolato sia prima sia dopo. Il collo
  di bottiglia UI era l'ordinamento/normalizzazione ripetuti delle 102 proprieta
  e le letture del profilo per controllo. Microbaseline dei soli ordini ripetuti:
  circa 35 ms/frame; getter dopo caching: 0,12–0,15 ms/frame. Sono misure di cicli
  diversi e non un rapporto di accelerazione dell'intero programma. Pannello
  grafico con 102 slider: massimo 2,049 ms per disegno, UI 90 FPS nel test locale.
- `tests/audio_integration.py`: audio reale da null sink privato, stereo L/R,
  silenzio, disabilitazione, reload, override web tipizzati e asset preset passati.
  Il test non cambia dispositivo predefinito. Prove in `local/ui10-audio-fixed/`.
- `tests/customization_integration.py`: 10 lingue via dropdown, 10 famiglie tema
  con entrambe le varianti, font normali e TTC, densita, shortcut e refresh passati
  (`local/ui10-style-tests/`). Il test densita controlli e refresh e passato anche
  in `local/ui10-performance-tests/`. Font names disegnati con cache di font propri;
  risorse caricate solo per righe visibili. Scroll interpolato in tutti i pannelli
  e griglia senza doppia interpolazione verticale. Refresh con due frecce curve.
- `tests/library_live_integration.py` passato dopo la correzione web: applicazione
  automatica, finestra persistente e renderer precedente conservato su errore.
- 9/9 CTest passati, inclusi dependency/cache/cicli, campi null e migrazione temi.
  Serie patch verificata anche da HEAD upstream fissato: i 46 file risultanti
  coincidono con i sorgenti locali. Hash manifest aggiornati; pin invariati.
- Installata 0.10.0 dalla build privata `build-ui10`; runtime precedente conservato,
  GUI precedente chiusa verificando identita. Ripristinato Cyberpunk 2077 City Loop
  dopo lo swap. Nessuna modifica all'avvio automatico. Evidenza installazione e
  controlli preferenze in `local/ui10-install/`.

## 2026-10-28 — Inspector, font e player audio 0.11.0

- Baseline privata: `local/ui11-baseline/`; pin upstream invariati. Patch nuova
  `engine-sound-controls.patch`, hash nel manifest. Applicazione completa da HEAD
  fissato verificata: i 60 file risultanti coincidono con l'albero locale.
- Identificato il preferito `3572814850`, 蕾塞reze. Due sound layer in modalita
  single/startsilent, comandati da un player SceneScript. Prima startsilent,
  volume, getChildren e trasporto non erano implementati: entrambe le canzoni
  partivano. Ora il player ne avvia una sola; nomi, trasporto, texture animate
  delle icone e vettori sono collegati al motore. Gli script CText non vengono
  piu eseguiti sia dal sistema generico sia dal lifecycle di testo.
- `tests/sound_controls_integration.py` passato con loop di clip corta, pausa,
  ripresa, stop, fine naturale e riavvio. Registrazione dal null sink controllata
  con due frequenze distinte, intervalli di silenzio e volumi diversi. Verificata
  anche la matematica Vec3.mix e l'uscita pulita. Prova con Reze e musica reale:
  una sola traccia attiva (in the pool), RMS circa 0,010 e picco 0,055;
  niente sovrapposizione. Evidenze in `local/ui11-playback-final/`.
- `tests/audio_integration.py` passato per FFT stereo, silenzio, reload,
  disabilitazione e override web; Audio Visualizer provato anche con la canzone
  JANE DOE estratta dalla copia Workshop locale. Spettro variabile, picco circa
  0,56, ritorno al silenzio dopo lo stop. Asset del preset web caricati.
  Evidenze in `local/ui11-visualizer-final/`. Spotify non e stato comandato:
  i test con vere canzoni usano gli audio gia presenti negli sfondi.
- Le prime catture erano silenziose perche SDL sceglieva esplicitamente il device
  di default ignorando PULSE_SINK. Il test ora specifica un device SDL dedicato,
  senza modificare il default. Un controllo di un solo istante sull'introduzione
  silenziosa della canzone e stato sostituito da campioni su quattro secondi.
- Provato il caso monitor coperto da applicazione fullscreen: uno stack eu-stack
  mostrava il blocco in EGL swap. Pacing per superficie e polling Wayland limitato
  eliminano quel blocco; pompa Chromium separata dalle callback di disegno.
  Catture e log privati includono i tentativi diagnostici non passati.
- Audit di avvio del catalogo attuale: 100/101 in single, con la sola scena 3D
  `3477054430` ancora esclusa come gia documentato. Provati inoltre 12/12 avvii
  mirror/span su Reze, Audio Visualizer e quattro scene con volume collegato alle
  proprieta (Berserk, Elden Ring, Chainsaw Man HDR, Jinx). Sono prove di readiness
  e breve sopravvivenza, non una certificazione di fedelta di ogni effetto.
  Renderer personale conservato durante le prove; identita verificata prima
  dello stop dei processi di test.
- `tests/inspector_controls_integration.py`: colore grafico salvato, zoom live
  durante drag e persistente al rilascio, reset creator che conserva posizione,
  popup display in Libreria/Carousel e salvataggi condivisi con impostazioni.
  Anteprima font verificata visivamente: famiglie e pesi distinguibili, indici
  delle raccolte rispettati. Evidenza finale `local/ui11-release-ui/`.
- `tests/customization_integration.py`: 10 lingue, entrambe le varianti dei temi,
  font TTF/TTC, layout, shortcut e refresh passati. Pannello di 102 slider: massimo
  2,499 ms per disegno durante altri test grafici, UI 70 FPS. Screenshot in
  `local/ui11-customization/`. 9/9 CTest passati, inclusa regressione volume
  JSON del creator e startsilent.
- Audit proprieta in `local/ui11-baseline/controls-audit.json`: 196 colori,
  589 slider, 334 bool, 119 combo; 29 controlli file, 9 directory e 2 scenetexture
  richiedono ancora selettori dedicati. Rilevate 144 condizioni con &&/|| che il
  parser di visibilita limitato non interpreta interamente. Questi punti restano
  follow-up, insieme alla persistenza di localStorage tra processi e alle
  proprieta che ricaricano la scena e possono quindi far ripartire la musica.
- Ulteriore osservazione sulla sessione, senza intervenire sulla configurazione:
  e presente anche il renderer esterno WallpaperX
  `~/.local/share/wallpaperx/backend/opt/linux-wallpaperengine/linux-wallpaperengine`
  (PID 1335 al controllo), con volume 0, panorama DP-5/DP-6 e 60 FPS.
  Consuma risorse in parallelo a Wallpaper Lab; non e stato modificato il suo
  processo ne il suo avvio. Alcuni processi zombie del vecchio controller GUI
  erano inoltre presenti dopo precedenti crash: da verificare il reaping dei
  renderer che escono dopo aver dichiarato readiness.

- Ultimo controllo del selettore: da un colore nero/grigio e possibile scegliere
  prima la tonalita e poi saturazione/luminosita senza perderla. La palette aperta
  scorre nel viewport per lasciare visibili i controlli. Prova pointer completa
  in `local/ui11-hsv-verified/`, inclusa sincronizzazione con la tab Display.

- Installata 0.11.0 dalla build privata verificata; runtime precedente conservato.
  Ripristinato Reze, riaperta la GUI. Preferenze personali confrontate prima/dopo
  e conservate. Nessuna modifica all'avvio automatico o al renderer WallpaperX.
  Evidenza di installazione e stato finale in `local/ui11-install/`.

## 0.12.0 — Workshop integrato, scorciatoie e impostazioni compatte (2026-10-28)

Implementazione Steam nativa ISteamUGC020, callback manuali, worker dedicato e
anteprime HTTPS libcurl in background. Nessun renderer o bundle WallpaperX viene
avviato. API e tag confrontati con la documentazione Steamworks e il catalogo
Workshop ufficiale di Wallpaper Engine; il bundle WallpaperX installato è stato
consultato per il flusso query/subscription/download.

Verifiche eseguite:

- **9/9 CTest**; nuovi casi di catalogo per AND/OR tra tipo, audio e risoluzioni,
  query vuota e risoluzione sconosciuta. Hash manifest e serie patch engine/raylib
  verificati; nessuna modifica ai pin o alle patch del renderer in questa fase.
- `tests/workshop_integration.py`: ricerca Steam reale, AND Scene+Video (0) contro
  OR (3873 al momento del test), tipo+1080p, autore+testo su tutte le sue pagine,
  ID/link diretto, metadati, pagina 2, rifiuto di ID non scaricabile. Il preset
  1806366520 riporta e scarica anche la dipendenza 893418273; callback e stato
  installato verificati. Evidenza: `local/ui12-steam-queries/`.
- Download reale di **Frieren | Sousou no Frieren**, 3100052627, 35.578.473 byte,
  e download tramite **un solo click nella UI** di **Minimal Sky**, 1818234419,
  217.520 byte. Sono vere sottoscrizioni Steam e rimangono nella libreria utente.
  Aggiornamento automatico osservato da 101 a 102 elementi. Nessuno dei due è
  stato applicato al desktop personale durante la prova Workshop.
- `tests/workshop_ui_integration.py`: Ctrl+L, UTF-8 (accenti, emoji, trattino),
  Ctrl+A/C/X/V e clipboard Wayland reale, ridimensionamento/persistenza del filtro,
  finestra impostazioni compatta, anteprime font, ricerca Steam, dettagli e
  download. Prima prova con download: `local/ui12-workshop-desktop/`; controlli
  successivi: `local/ui12-workshop-final/` e `local/ui12-workshop-cjk/` (90 FPS
  osservati in quelle finestre; non è una garanzia di FPS per ogni scena).
- `tests/active_delete_integration.py`: fixture web native, config/state/Trash
  isolati. Eliminazione attivo → primo altro preferito, carousel vuoto → prima
  voce libreria, ultimo wallpaper → stop sicuro, metadata del cestino recuperabile,
  sostituto non avviabile → PID/start/file originali invariati. Errore del cestino
  simulato sull'ultimo wallpaper → renderer ripristinato. Tutti passati:
  `local/ui12-delete-final/`.
- `tests/customization_integration.py`, aggiornato ai dropdown: **10 lingue**,
  **10 temi × 2 varianti**, densità, font TTF/TTC, peso e reload senza riavvio,
  shortcut inline condiviso con le impostazioni, refresh che conserva selezione,
  preferiti e renderer. 102 proprietà creator: massimo 0,767 ms, 90 FPS osservati
  nel test isolato. `local/ui12-customization-final/`.
- `tests/inspector_controls_integration.py`: HSV, zoom live/persistente, reset che
  conserva posizione, popup display dedicato da Libreria e Carousel, salvataggio
  condiviso con le impostazioni e anteprime font. Passato:
  `local/ui12-inspector-final/`.
- Chiusura ordinata del browser Workshop durante richieste/cache attive:
  `--workshop-ui-test`, exit 0, senza SIGTERM: `local/ui12-workshop-shutdown/`.
- Catalogo personale dopo i download: **102 elementi, 101 riconosciuti/supportati**;
  resta la scena prospettica 3D 3477054430 già documentata. Questo conteggio non
  equivale a un nuovo audit di rendering completo dei 102 progetti.

Limiti dichiarati:

- La ricerca globale dei display name Steam attraverso l'endpoint Community
  anonimo richiede login (HTTP 401). L'app suggerisce autori incontrati e accetta
  nome noto, vanity URL, profilo o SteamID64; gli omonimi non vengono scelti in
  silenzio. “Altri sfondi di questo autore” usa lo SteamID esatto. Nessun cookie
  o credenziale viene estratto dal browser.
- Download e sottoscrizioni usano Steam/Wallpaper Engine posseduto dall'account;
  non c'è download anonimo né sostituzione del sistema di licenze Steam.
- Il supporto del renderer non cambia con il download: applicazioni Windows e
  scene prospettiche 3D possono restare incompatibili. Anteprime Workshop statiche,
  non rendering remoto di codice non ancora scaricato.
- `schemecolor`: 92 progetti installati lo dichiarano; nessun riferimento diretto
  trovato nei rispettivi file scena. Non prova l'assenza di ogni uso via script.
  La UI spiega che l'effetto dipende dal collegamento fatto dal creator, e che
  non è il tema dell'app. Riferimento ufficiale:
  https://docs.wallpaperengine.io/en/scene/userproperties/color.html
- Le prove usano namespace XDG separati, fixture eliminabili e Steam per i soli
  download autorizzati. Nessuna modifica all'avvio automatico, ai dispositivi
  audio predefiniti o a processi WallpaperX esterni.

Ultimo controllo 0.12: clic reale sulla stessa card mentre l'eliminazione attiva
è in corso (GIO rallentato soltanto nel test). La richiesta non viene rimessa in
coda dopo aver eliminato il progetto: caricamento completato, sfondo Beta attivo,
UI sbloccata. `tests/delete_ui_integration.py`, `local/ui12-delete-ui/`.

## 0.13.0 — menu, monitor, media e transizioni

- CTest: 9/9; i18n verifica 22 locale e formati dei 279 messaggi (nuove lingue:
  menu principali tradotti manualmente, messaggi avanzati ancora inglesi).
- `independent_monitors_integration.py`: due renderer web su uscite distinte;
  sostituzione del primo lascia identico PID/start del secondo; stop/restore
  ripristina entrambe le assegnazioni. Trace OpenGL iris: avvio e completamento.
  Evidenza `local/ui13/monitor-final/`.
- `media_metadata_integration.py`: sessione DBus privata, player MPRIS simulato
  già presente prima dell'avvio. Titolo giapponese/coreano/hindi/arabo ricevuto,
  posizione 30 s e durata 180 s corrette da 30/180 milioni di microsecondi;
  costante PLAYBACK_PLAYING disponibile. Nessuna modifica ai player personali.
  Evidenza `local/ui13/media-final/`. Questa prova verifica il contratto MPRIS,
  non un nuovo test di riproduzione musicale reale da Spotify/YouTube.
- Prova mouse/tastiera: menu risoluzioni sovrapposto senza spostare i campi,
  click bloccati sui controlli coperti, scelta araba con shaping corretto,
  filtro Workshop applicato automaticamente (revision incrementata senza Search),
  dropdown distinti del carousel e anteprima separata. `local/ui13/gui/`.
- Serie patch verificata in reverse sul vendor, pin invariati.

Limiti: nuove localizzazioni parziali e layout RTL non specchiato; dipendenza dai
font installati. Le transizioni hanno durata iniziale 700 ms e usano snapshot
nativi del solo wallpaper; se il renderer è sospeso e non produce lo snapshot,
il cambio avviene senza effetto. Nessuna importazione dei preferiti Spotify.

Verifica aggiuntiva Reze 3572814850 con MPRIS isolato già attivo: i due script
CText del progetto reale ricevono titolo `日本語 한국어 हिन्दी العربية` e artista
`Artist`; la texture dei testi viene ricostruita. È stato corretto anche il
registro separato `__textLayers`, inizialmente non raggiunto dagli eventi media.
Evidenza: `local/ui13/reze-test/`, senza modificare file Workshop o player utente.

## 0.14.0 — transizioni, Xray, profili e navigazione (29 ottobre 2026)

- Build app e libreria motore riuscite; CTest **10/10**, compreso controllo delle
  chiavi UI nel catalogo (305 messaggi × 22 lingue, formati printf preservati).
- `transition_graphics_integration.py`: otto effetti su renderer embedded reale,
  25 frame per effetto. Snapshot rosso/verde e nuova scena web asimmetrica:
  orientamento precedente/successivo corretto; partenza dopo readiness e termine
  tracciati. Nessun nero inatteso negli effetti che non prevedono il nero.
  Saturazione, contrasto e luminosità live, persistenza e reset senza cambio PID;
  il renderer personale è rimasto identico. `local/ui14/transitions/`.
- `text-cache-graphics-test`: 1.100 testi Unicode/fotogramma, oltre i 1.024 slot LRU.
  Rendering in batch confrontato con flush immediato: **0 pixel differenti**,
  187.909 pixel di testo effettivamente renderizzati. `local/ui14/text-cache/`.
- Xray 3257420414: proprietà reali nella GUI, 24 catture durante scrolling avanti
  e indietro; nessun flash bianco nel pannello filtri. Scroll centrale e thumb
  del menu lingue trascinati con input reale. `local/ui14/xray/`.
  Questa prova della GUI non applica lo sfondo personale.
- Lo stesso Xray è stato anche renderizzato con un processo isolato su DP-6:
  luminosità +15% applicata senza riavvio, intensità media da 42,21 a 79,19,
  nessun frame nero nella sequenza misurata. `local/ui14/xray-render/`.
- `playback_options_integration.py`: tutte le quattro combinazioni globale/locale
  per cinque opzioni nei launch plan; globale OFF ha sempre precedenza.
- `ui14_integration.py`: 205 progetti web fittizi, pagina 1/2 reale, 50/100/200
  risultati, categoria risoluzione intera con AND, overlay senza spostare i campi,
  tre slider immagine persistenti, cinque dropdown locali persistenti, splitter
  filtri, opacità indipendente, cambio di tutte le 22 lingue e preview transizione
  correttamente orientata. `local/ui14/demo-check/`.
- Steam reale, sola lettura: nome/autore, AND/OR, dettagli e dipendenze, gruppo
  risoluzioni OR dentro AND con tipo Scene, 100/200 risultati, due pagine da 100
  senza ID duplicati. `local/ui14/steam/`, `local/ui14/steam-more/`.
- GUI Workshop reale: filtri tipo e categoria risoluzione applicati automaticamente,
  cambio a 200 risultati senza Search, miniature con azione download; Esc chiude
  il menu senza lasciare Workshop. `local/ui14/workshop-ui/`.
  In questa versione non è stata avviata una nuova sottoscrizione da una miniatura;
  la stessa API di download è già stata verificata con sottoscrizioni reali in 0.12.
- Serie patch completa e hash manifest verificati, pin upstream invariati.

Limiti: traduzioni complete nel catalogo ma senza revisione madrelingua esterna;
layout RTL ancora non specchiato. Metadati Steam e testo libero dei creator
rimangono originali. Correzione immagine finale SDR, non color grading HDR.
Non è un nuovo audit grafico di tutti i wallpaper: eventuali artefatti specifici
di altri progetti richiedono identificazione e riproduzione. Restano i limiti di
compatibilità del renderer documentati nelle versioni precedenti. Nessuna modifica
all'autostart, ai player/dispositivi audio personali o a processi WallpaperX.

Comandi riproducibili (test grafici da eseguire con schermo sbloccato):

```sh
ctest --test-dir build-ui10 --output-on-failure
build-ui10/text-cache-graphics-test "$PWD/local/text-cache-test"
python3 tests/ui14_integration.py --app local/ui14-runtime/wallpaper-lab --output local/ui14-check
python3 tests/transition_graphics_integration.py --app local/ui14-runtime/wallpaper-lab --output local/transition-check
python3 tests/playback_options_integration.py --app local/ui14-runtime/wallpaper-lab --output local/playback-check
python3 tests/workshop_integration.py --app local/ui14-runtime/wallpaper-lab --output local/workshop-check
```

Installazione locale 0.14 completata con backup automatico del runtime precedente.
Ripristinati entrambi i renderer indipendenti DP-5/DP-6 con gli stessi wallpaper.
Carousel e profili personali identici prima/dopo; nelle impostazioni sono stati
aggiunti soltanto i nuovi default `page_size=100` e `window_opacity=100`.
Evidenza privata `local/ui14/install/`.
