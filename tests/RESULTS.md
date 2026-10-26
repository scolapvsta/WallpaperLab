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
