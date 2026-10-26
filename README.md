# Wallpaper Lab

Nome provvisorio di una nuova applicazione Linux: il selettore animato derivato
da Hellpaper e il renderer di linux-wallpaperengine nello stesso progetto.

Il programma **non esegue Hellpaper, WallpaperX o il binario linux-wallpaperengine**.
Compila una propria interfaccia e collega direttamente la libreria del motore,
ricompilata dai sorgenti. Un unico eseguibile, `wallpaper-lab`, gestisce selettore,
anteprime e rendering. Il wallpaper continua in una seconda istanza dello stesso
eseguibile quando il selettore si chiude. Questo permette di isolare il rendering
e lasciare libera la memoria dell'interfaccia.

## Prima versione

- Quattro layout Hellpaper: griglia, fiume orizzontale, fiume verticale e onda,
  con animazioni di posizione, dimensione, hover e apertura/chiusura anteprima.
- Ricerca per titolo, miniature e tipo del progetto.
- Rilevamento delle librerie Steam standard, Flatpak e delle librerie aggiuntive
  elencate in `libraryfolders.vdf`; lettura dei progetti Workshop `431960`.
- Anteprime GIF animate con i tempi originali dei frame, caricate una alla volta.
- Rendering della scena completa o del video in una finestra di anteprima.
- Applicazione panoramica, duplicata o su un singolo monitor Hyprland; audio e FPS.
- Nuovo wallpaper avviato prima di fermare il precedente. Se il caricamento
  fallisce, il precedente rimane in esecuzione. Lock fra operazioni concorrenti;
  verifica PID, tempo d'avvio e percorso dell'eseguibile prima di inviare segnali.
- Configurazione persistente, ripristino all'accesso e launcher nel menu.

È una prima versione funzionante, non una riproduzione completa di Wallpaper Engine.
Il renderer determina la compatibilità degli effetti delle scene. I preset che
dipendono da un altro progetto sono riconosciuti e mostrati, ma non applicati.
Il codice del backend supporta progetti web; non sono stati verificati in questa
sessione. Il supporto a immagini/GIF autonome fuori dal Workshop è futuro.
Le transizioni conservate sono quelle del **selettore**: non è implementata una
dissolvenza fra due scene sul desktop. Il caricamento delle anteprime GIF ha un
limite di 128 MiB decodificati e 512 frame; oltre il limite si usa il fotogramma statico.

## Uso

Aprire **Wallpaper Lab** dal menu applicazioni, oppure eseguire `wallpaper-lab`.

| Comando nel selettore | Azione |
| --- | --- |
| 1–4 o pulsanti in alto | Cambia layout |
| Rotella / Ctrl+rotella | Scorri / cambia dimensione miniature |
| Frecce o HJKL | Seleziona |
| `/`, testo, Invio | Cerca per titolo |
| Invio o clic sinistro | Applica il wallpaper e chiude il selettore dopo l'avvio |
| Tasto destro o Shift sinistro | Ingrandisce l'anteprima Workshop; GIF animate |
| Spazio | Apre la scena completa in una finestra; ESC la chiude |
| M / Tab | Modalità monitor / monitor in modalità singola |
| F / V | 30, 60 o 120 FPS / audio on-off |
| ESC | Chiude ricerca, anteprima o selettore |

Le impostazioni del renderer vengono salvate quando si applica un wallpaper.
Nella versione iniziale la modalità singola sostituisce il gruppo precedente:
non è ancora un editor di assegnazioni diverse per ogni monitor.

Comandi disponibili:

```sh
wallpaper-lab --list            # catalogo JSON
wallpaper-lab --plan 3144757256  # argomenti del renderer senza avviarlo
wallpaper-lab --preview 3144757256
wallpaper-lab --apply 3144757256
wallpaper-lab --status
wallpaper-lab --restore
wallpaper-lab --stop
```

Il comando `--preview` lascia aperta la finestra; ESC o la chiusura del compositor
la terminano. Le anteprime aperte dal selettore vengono chiuse quando esce.

## Compilazione e manutenzione

Sorgenti C/C++20, CMake e Ninja. La libreria del motore viene compilata separatamente,
poi collegata all'app. Le dipendenze del motore sono quelle documentate nel suo
README, incluse SDL2, GLFW >=3.4, GLEW, GLM, MPV, FFmpeg, PulseAudio, FreeType, DBus,
LZ4, OpenGL e protocolli Wayland. Raylib 5.5 è inclusa come submodule e compilata
staticamente, usando lo stesso GLFW del renderer. CEF viene scaricato e verificato
dal sistema di build upstream; richiede spazio e una connessione di rete.

```sh
./scripts/build.sh
python scripts/install.py
```

Su questo PC le dipendenze mancanti sono in `local/sysroot/usr`, ricavate da
pacchetti Arch già presenti e dalle librerie del runtime precedente. Nessun
pacchetto di sistema è stato installato o aggiornato. Su una macchina nuova usare
le dipendenze di sviluppo di sistema, oppure preparare esplicitamente quel prefix.
`local/` è esclusa da Git: la compilazione da clone pulito presuppone tali dipendenze.

I submodule fissano revisioni precise. Le patch in `patches/` sono applicate dal
build solo se mancanti e verificabili: non si sovrascrivono modifiche sconosciute.
Dopo il build `vendor/engine` e `vendor/raylib` risultano modificati dalle patch:
è previsto. Per cambiare il motore, modificare i sorgenti, aggiornare la patch e
verificare di nuovo. Le modifiche al selettore si fanno direttamente in `src/selector.c`.

L'installer prepara un runtime proprio sotto `~/.local/share/wallpaper-lab/runtime`,
un launcher `~/.local/bin/wallpaper-lab` e una voce desktop. Conserva il runtime
precedente per rollback. Se il renderer di questa app era attivo, lo ferma e lo
ripristina dopo l'aggiornamento. Chiudere il selettore prima di reinstallare.

Configurazione: `~/.config/wallpaper-lab/settings.json` e `selector.conf` opzionale.
Stato e log: `~/.local/state/wallpaper-lab/`. Il nuovo runtime non usa percorsi di
WallpaperX o della directory di build. Gli asset originali restano in Steam.

## Migrazione e rollback

`scripts/takeover-wallpaperx.py` è una migrazione esplicita e una tantum per un
wallpaper comune ai monitor. Salva i file precedenti sotto `local/before-takeover-*`,
importa la scena attiva, avvia il nuovo renderer e solo dopo arresta i processi
WallpaperX rilevati nel percorso utente. Sostituisce l'autostart con
`~/.config/autostart/wallpaper-lab.desktop` (`wallpaper-lab --restore`).

Per tornare a WallpaperX: eseguire `wallpaper-lab --stop`, rimuovere l'autostart di
Wallpaper Lab, ripristinare `original-autostart.desktop` dal backup al nome
`~/.config/autostart/Linux Wallpaper Engine.desktop`, quindi avviare `wallpaperx`.
I file della libreria Steam e l'installazione WallpaperX rimangono disponibili.

## Verifiche

```sh
ctest --test-dir build --output-on-failure
python tests/integration.py --output local/integration.json
# Questo test cambia davvero il wallpaper, poi ripristina la scelta precedente:
python tests/integration.py --desktop --output local/integration-desktop.json
# Test grafico: si apre brevemente una finestra; screenshot esclusi da Git:
mkdir -p local/screenshots
WALLPAPER_LAB_TEST_OUTPUT="$PWD/local/screenshots" wallpaper-lab --ui-test
```

Risultati e limiti della sessione iniziale in `tests/RESULTS.md`.
Il repository Git è locale; nome definitivo e destinazione GitHub sono da decidere.
Attribuzioni e licenze in `THIRD_PARTY.md` e `LICENSE`.
