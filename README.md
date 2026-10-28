# Wallpaper Lab

Nome provvisorio di una nuova applicazione Linux: il selettore animato derivato
da Hellpaper e il renderer di linux-wallpaperengine nello stesso progetto.

Il programma **non esegue Hellpaper, WallpaperX o il binario linux-wallpaperengine**.
Compila una propria interfaccia e collega direttamente la libreria del motore,
ricompilata dai sorgenti. Un unico eseguibile, `wallpaper-lab`, gestisce selettore,
anteprime e rendering. Il wallpaper continua in una seconda istanza dello stesso
eseguibile quando il selettore si chiude. Questo permette di isolare il rendering
e lasciare libera la memoria dell'interfaccia.

## Finestra principale e popup carousel (0.5.0)

L’app rimane aperta dopo l’applicazione di uno sfondo. Un clic su una miniatura
la seleziona; il pulsante **Applica** cambia il desktop. La finestra può essere
chiusa senza interrompere il renderer. Il testo usa un font di sistema tramite
Fontconfig e filtraggio bilineare.

- **Libreria**: miniature piu grandi con titolo su due righe, bordi bianchi,
  stella bianca vuota/piena e cestino su ogni progetto. La selezione apre un
  pannello laterale integrato: anteprima del progetto, dettagli e azioni. La
  griglia si ricompone sulla sinistra; i comandi principali restano visibili.
- **Ricerca**: campo centrale con cursore, cancellazione e filtri combinabili
  per tipo, risoluzione e reattivita audio. Le dimensioni vengono lette dalla
  scena (anche `scene.pkg`) o dal video tramite `ffprobe`, se disponibile.
  Le miniature non vengono usate per indovinare la risoluzione. I metadati
  mancanti si trovano con **Da verificare**. Il filtro audio usa il supporto
  dichiarato dal progetto, lo stesso letto dal motore.
- **Controlli a destra**: filtri, luna (attiva = scuro, disattiva = chiaro) e
  rotella delle impostazioni. Le modifiche al tema vengono salvate subito.
- **Carousel**: gestione con gli stessi componenti della libreria, stella
  bianca e cestino, riordino e cambio precedente/successivo nel pannello laterale.
  La rotella apre direttamente le impostazioni del carousel.
- **Impostazioni**: Display, Riproduzione, Carousel e Aspetto sono raccolti
  sia nella schermata principale sia nel popup. Panoramico, duplicato o singolo
  monitor; risoluzioni automatiche, FPS, audio ed effetti. La modalita singola
  sostituisce il gruppo attivo e non crea sfondi diversi simultanei per monitor.
- **Carousel → Seleziona shortcut**: premi la combinazione completa per
  salvarla e attivarla, oppure Esc per annullare. Non occorre scegliere i tasti
  da una lista. Lo shortcut attuale viene sospeso in memoria durante la
  registrazione e ripristinato all'annullamento o alla chiusura normale.
  La configurazione e salvata con backup e controllo dei conflitti.
- **Cestino**: richiede un secondo clic dopo il rilascio del primo. I file
  vengono spostati nel cestino del sistema e rimossi dal carousel. Lo sfondo
  attivo e le scene live aperte sono protetti; Steam puo riscaricare un progetto
  al quale si e ancora iscritti. Non esiste cancellazione permanente automatica.

L'identita visiva e provvisoria: icona vettoriale propria, Adwaita Sans per il
contenuto, DejaVu Sans Bold per i pulsanti e Noto Serif per il nome. I font sono
risolti con Fontconfig; FreeType gestisce il fallback per i titoli in CJK usando
solo i glifi presenti nella libreria. Nessun font di sistema viene copiato in repo.

Nei layout Fiume, Verticale e Onda un unico ingrandimento segue la posizione
animata della striscia. Le ripetizioni rapide di Super+Tab non avviano una nuova
animazione di scala indipendente su ogni sfondo. Le copie ai bordi usano la stessa
geometria e lo spazio trasparente accoglie l’ingrandimento.

Il popup mostra solo le immagini, senza intestazione o footer. Ogni miniatura
ha un bordo bianco di circa 2,6 pixel, uguale per tutte; la selezione si distingue per
l'ingrandimento animato. **Griglia**, **Fiume** e **Verticale** hanno angoli esterni
arrotondati e immagini accostate senza spazi fra loro.
La **griglia** si adatta al numero di preferiti: quattro formano un 2×2,
nove un 3×3, con celle da 160 pixel logici. Le liste grandi sono suddivise in
pagine, mantenendo visibile la selezione e rispettando le dimensioni del monitor.
Nella griglia la scelta cresce sopra il mosaico e oltre i suoi bordi, mentre
le celle sottostanti restano piene. **Fiume** e **Verticale** sono strisce di
miniature quadrate da 160 pixel: le altre immagini mantengono dimensioni uguali,
mentre quella scelta si ingrandisce in entrambe le direzioni e sporge dal gruppo.
I contorni sono decorativi e non tagliano la selezione. Una fascia trasparente
attorno al gruppo offre lo spazio necessario allo zoom. **Onda** usa uno sfondo trasparente:
si vedono immagini con angoli morbidi e bordi bianchi, senza cornice esterna.
Lo zoom è più marcato e le miniature vengono decodificate a 256 pixel per
mantenere una maggiore nitidezza durante l'ingrandimento.

Entrata e uscita usano la stessa dissolvenza di 140 ms, riprodotta al contrario
in chiusura. Un rilascio durante l'apertura inverte la dissolvenza dal punto
raggiunto. Quando si passa da un'immagine all'altra, quella scelta cresce e la
precedente torna alla dimensione normale, accompagnando lo scorrimento. Il
movimento è più graduale (circa 350 ms per la maggior parte del percorso) e
conserva la velocità fra pressioni successive, continuando in avanti anche
quando si ritorna al primo elemento. Una nuova pressione
durante la chiusura resta disponibile per la sessione successiva.
L'animazione è gestita dall'app, senza sommare quella del compositor.
Il popup legge solo i progetti preferiti, decodifica le miniature durante la
creazione della finestra e non carica il font della finestra principale.
Il primo avvio resta soggetto al tempo di creazione della finestra e del contesto
grafico; il caricamento della scena desktop avviene sempre dopo la conferma.

La scorciatoia del popup è integrata con Hyprland con configurazione standard
**Lua**. L’app verifica i conflitti, salva un backup in
`~/.local/state/wallpaper-lab/backups/`, aggiunge un include del proprio file
`shortcut.lua` e ricarica solo la configurazione. **Disattiva**
rimuove il binding dal file gestito; il piccolo include resta riutilizzabile.
La scorciatoia non viene attivata automaticamente su altre installazioni.
Su altri compositor si può associare manualmente `wallpaper-lab --next` per il
cambio diretto, senza popup. Con combinazioni personalizzate, il rilascio di un
modificatore della combinazione conferma. Le richieste delle applicazioni di
inibire le scorciatoie globali vengono rispettate.
Le API utilizzate sono documentate nella [guida Hyprland ai binding](https://wiki.hypr.land/configuring/core/binds/).

Un carousel vuoto restituisce un messaggio senza cambiare il desktop. Elementi
rimossi dalla libreria o non supportati sono saltati. Se il rendering del
successivo fallisce, il precedente rimane attivo. Durante un cambio le richieste
concorrenti vengono rifiutate senza accodare altri renderer.

## Funzionalità del renderer e del catalogo

- Quattro layout Hellpaper nel popup carousel: griglia, fiume orizzontale,
  fiume verticale e onda, con transizioni rapide e selezione evidenziata.
- Ricerca per titolo, miniature e tipo del progetto.
- Rilevamento delle librerie Steam standard, Flatpak e delle librerie aggiuntive
  elencate in `libraryfolders.vdf`; lettura dei progetti Workshop `431960`.
- Anteprime GIF animate con i tempi originali dei frame, caricate una alla volta.
- Rendering della scena completa o del video in una finestra di anteprima.
- Applicazione panoramica, duplicata o su un singolo monitor Hyprland; audio e FPS.
- Reazione alla musica separata dal suono del wallpaper. Cattura stereo del monitor
  dell'uscita PulseAudio/PipeWire, con spettro per shader, SceneScript e wallpaper web.
  “Suono dello sfondo” può restare disattivato mentre “Reazione alla musica” è attiva.
  Riapplicare il wallpaper dopo aver cambiato queste opzioni.
- Nuovo wallpaper avviato prima di fermare il precedente. Se il caricamento
  fallisce, il precedente rimane in esecuzione. Lock fra operazioni concorrenti;
  verifica PID, tempo d'avvio e percorso dell'eseguibile prima di inviare segnali.
- Configurazione persistente, ripristino all'accesso e launcher nel menu.

È una prima versione funzionante, non una riproduzione completa di Wallpaper Engine.
Il renderer determina la compatibilità degli effetti delle scene. I preset che
dipendono da un altro progetto sono riconosciuti e mostrati, ma non applicati.
I progetti web sono stati verificati con “Audio Visualizer” (893418273), inclusa
la risposta stereo alla musica e la chiusura dei processi Chromium. Il supporto a immagini/GIF autonome fuori dal Workshop è futuro.
Le transizioni conservate sono quelle del **selettore**: non è implementata una
dissolvenza fra due scene sul desktop. Il caricamento delle anteprime GIF ha un
limite di 128 MiB decodificati e 512 frame; oltre il limite si usa il fotogramma statico.

Le estremita dell’onda sono chiuse da bordi bianchi laterali, applicati
all’intero gruppo durante la composizione.

## Disposizione fisica dei monitor (0.6.0)

Nella rotella → **Display** la mappa mostra i monitor in proporzione alle
misure del pannello, con risoluzione nativa e dimensioni in centimetri. Trascina
un monitor per spostarlo; l’aggancio ai bordi aiuta l’allineamento e **Alt** lo
sospende. **Allinea in alto / Al centro / In basso** usano il pannello piu alto
come riferimento. I monitor sovrapposti non si possono salvare.

**Dimensioni reali** compensa la densita diversa dei pixel. Su questo setup il
1080p a sinistra e circa 53×30 cm e il QHD a destra 60×33 cm: il primo ha quasi
la stessa altezza fisica, non tre quarti. Le dimensioni vengono rilevate dal
monitor; quando mancano sono esplicitamente indicate come stimate. **Pixel
logici** permette di tornare al criterio del desktop. **Posizioni rilevate**
ripristina la geometria rilevata all’apertura; **Annulla** ricarica quella salvata.

**Salva disposizione** conserva il profilo per i prossimi sfondi panoramici;
**Salva e applica** riavvia lo sfondo corrente con la nuova calibrazione. Le
impostazioni precedenti vengono conservate in un backup XDG. Il renderer riceve
una fotografia immutabile del profilo, identificata per nome del connettore:
le superfici mantengono le risoluzioni native. Le assegnazioni di finestre,
il percorso del cursore e la configurazione monitor di Hyprland rimangono invariati.
La calibrazione si applica al panorama; singolo monitor e duplicato non la usano.

Comandi di verifica: `wallpaper-lab --display-layout` mostra geometria e misure;
`wallpaper-lab --display-layout-save physical top` salva il profilo fisico allineato
in alto con backup. `wallpaper-lab --display-settings` apre direttamente la mappa.

## Uso

Aprire **Wallpaper Lab** dal menu applicazioni, oppure eseguire `wallpaper-lab`.
Un clic applica lo sfondo alla destinazione configurata e apre la scena live a destra; la X richiude il pannello. La rotellina scorre
indipendentemente la griglia e il contenuto del pannello.

| Controllo | Azione |
| --- | --- |
| Clic su una miniatura / frecce | Seleziona, applica e mostra la scena live |
| Separatore della scheda destra | Trascina per ridimensionare la scena e la griglia |
| Slider della posizione | Aggiorna il renderer attivo e salva al rilascio |
| Stella bianca | Aggiunge o rimuove dal carousel |
| Precedente / Successivo | Applica il precedente/successivo preferito |
| Rotella / barra di scorrimento | Scorre il layout |
| Ctrl+rotella | Cambia la dimensione delle miniature |
| Carousel → rotella → Stile dello switcher | Sceglie il layout animato del popup |
| Windows tenuto premuto + Tab ripetuto | Apre il popup e avanza fra i preferiti |
| Rilascio di Windows / Esc nel popup | Conferma / annulla la scelta |
| Cerca / `/` | Cerca per titolo |
| ESC | Annulla registrazione, chiude popup, ricerca o pannello laterale |
| Chiusura della finestra | Chiude l’interfaccia, lascia il wallpaper attivo |

Le impostazioni del renderer vengono salvate quando si modificano i controlli.
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
wallpaper-lab --carousel        # lista ordinata dei preferiti
wallpaper-lab --favorite ID     # aggiunge/rimuove un preferito
wallpaper-lab --next            # applica il successivo
wallpaper-lab --previous        # applica il precedente
wallpaper-lab --shortcut SUPER+Tab
wallpaper-lab --shortcut-off
```

Il comando `--preview` lascia aperta la finestra; ESC o la chiusura del compositor
la terminano. La scheda live della libreria usa il renderer desktop, senza aprire una seconda scena.

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
python tests/carousel_integration.py --desktop --output local/carousel-integration.json
# Richiede una sessione Hyprland, shortcut attiva, almeno due preferiti e
# YDOTOOL_SOCKET configurato; esercita la tastiera e ripristina il desktop:
python tests/popup_integration.py --desktop --output local/popup-tests/results.json
# Verifica grafica isolata: non inietta tasti e non cambia il renderer desktop.
python tests/popup_visual_integration.py --desktop --output local/popup-visual
# Se il desktop è coperto, esporta anche il canvas dell'app prima del fade:
python tests/popup_visual_integration.py --desktop --capture-canvas --output local/popup-canvas
# Test grafico: si apre brevemente una finestra; screenshot esclusi da Git:
mkdir -p local/screenshots
WALLPAPER_LAB_TEST_OUTPUT="$PWD/local/screenshots" wallpaper-lab --ui-test
# Aggiungere WALLPAPER_LAB_TEST_APPLY=1 verifica anche una vera applicazione
# dello sfondo attuale e la permanenza della finestra dopo il caricamento.
```

Test audio con un'uscita virtuale silenziosa (non cambia i dispositivi predefiniti),
fixture web/SceneScript, reload web, avvio fallito e shutdown:

```sh
python tests/audio_integration.py \
  --app ~/.local/share/wallpaper-lab/runtime/wallpaper-lab \
  --assets ~/.local/share/Steam/steamapps/common/wallpaper_engine/assets \
  --workshop ~/.local/share/Steam/steamapps/workshop/content/431960/893418273 \
  --output local/audio-integration
```

Richiede Node 22+, `pactl`, `paplay` e una sessione grafica. `--workshop` è opzionale;
la prova di quel progetto usa il suo callback `wallpaperAudioListener`. I campioni
sono sintetici e finiscono soltanto nell'uscita virtuale. Per diagnosi si può forzare
il monitor con `WALLPAPER_LAB_AUDIO_MONITOR=nome.monitor`; normalmente viene seguito
il dispositivo di uscita predefinito. Il microfono non è la sorgente predefinita.

La compatibilità resta parziale: alcuni operatori audio delle particelle e altre
API avanzate del motore upstream non sono implementati. I preset dipendenti da
altri progetti restano esclusi. La calibrazione dello spettro non è identica alla
versione Windows di Wallpaper Engine.

Riferimenti dell'integrazione: [crash CEF e ordine di link](https://github.com/Almamu/linux-wallpaperengine/issues/628),
[API audio web](https://docs.wallpaperengine.io/en/web/audio/visualizer.html),
[AudioBuffers SceneScript](https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/AudioBuffers.html).

Test aggiuntivi della configurazione della scorciatoia, con compositor simulato:

```sh
python tests/shortcut_integration.py ~/.local/share/wallpaper-lab/runtime/wallpaper-lab
```

Risultati e limiti delle sessioni in `tests/RESULTS.md`.
Repository privata: https://github.com/scolapvsta/wallpaper-engine-customized.
Il nome definitivo dell'applicazione resta da scegliere.
Attribuzioni e licenze in `THIRD_PARTY.md` e `LICENSE`.

## Direzione concordata

Prima consolidare la finestra principale, l’uso col mouse, le impostazioni e il
carousel di preferiti con scorciatoia. La revisione più ampia della UI verrà
iterata con Simone. La prima integrazione Workshop nativa è disponibile dalla
versione 0.12: usa Steam UGC direttamente, senza avviare WallpaperX.

## Libreria, filtri e scena live (0.8.0)

Il pannello filtri si apre a sinistra. Tipo e audio restano combinabili; le
risoluzioni comuni sono organizzate in categorie espandibili: monitor singolo,
dual, ultrawide, triplo e verticale. La lista segue le
[risoluzioni ufficiali di Wallpaper Engine](https://docs.wallpaperengine.io/en/scene/performance/resolution.html),
senza aggiungere ogni dimensione insolita trovata nella libreria. Piu risoluzioni
selezionate richiedono che il progetto le dichiari tutte (AND).

La scansione legge la dimensione nativa, i tag di risoluzione disponibili e le
opzioni delle proprieta di tipo combo esplicitamente dedicate alla risoluzione.
Non vengono dedotte varianti dalla descrizione o dalla sola possibilita di scalare
un'immagine. Le proprieta di risoluzione dichiarate dall'autore sono selezionabili
nella scheda destra e la scelta viene passata al motore con `--set-property`.
I tag usati solo per la ricerca non sono presentati come file alternativi.
Nella libreria attuale non risultano proprieta di risoluzione selezionabili:
la scheda mostra quindi originale/automatica, senza creare varianti fittizie.

Un clic seleziona e applica lo sfondo alla destinazione delle impostazioni
(panoramico, duplicato o singolo monitor). I clic durante un caricamento conservano
l'ultima scelta in coda. Il renderer precedente resta attivo finche il nuovo
non e pronto. Il pulsante Applica rimane nelle impostazioni generali/display.

La scena live a destra e una vista del monitor selezionato, ricavata dallo stesso
renderer desktop: massimo 640×360, fino a 12 aggiornamenti/s. La vista riempie
il riquadro ritagliando l'eccedenza; non aggiunge bande da letterboxing.
La richiesta scade dopo due secondi senza interfaccia, interrompendo lettura GPU
e trasferimento dei fotogrammi. Durante la vista live il renderer continua anche
se una finestra a schermo intero normalmente lo metterebbe in pausa.
Trascinando il separatore si ridimensiona la scheda; la griglia restringe le
miniature fino a 210 px e poi riduce le colonne. Ogni tessera raggruppa miniatura,
titolo e metadati su un proprio fondo.

Gli slider orizzontale e verticale aggiornano il renderer attivo durante il
trascinamento, senza riavviarlo. Al rilascio salvano il profilo; «Centra lo sfondo»
azzera entrambi. Lo spostamento usa il margine del ritaglio disponibile; lo zoom permette
di aumentarlo senza scoprire il canvas. In panoramico il canvas si sposta insieme, mantenendo la giunzione.
I valori e la variante sono salvati per percorso in
`~/.config/wallpaper-lab/wallpaper_profiles.json`, separati dai preferiti e con
backup prima delle modifiche persistenti. Cambiare sfondo, rimuoverlo/rimetterlo
nel carousel o riavviare conserva la posizione.

Comandi: `--position ID [X Y]` legge/salva valori da -1 a 1;
`--variant ID [INDICE]` legge/salva la variante (-1 per l'originale).

Nel carousel i bordi arrotondati mantengono lo stesso raggio durante lo zoom.
La griglia mostra una stella e «Spazio libero» nelle celle vuote; Fiume e Verticale
usano la stessa distanza per calcolo e disegno, contenendo tre immagini complete
quando il movimento si ferma. L'occhio continua ad aprire il vero switcher in
modalita dimostrativa, senza cambiare il wallpaper.

## Scene interattive e lente X-Ray

Il renderer gestisce il padding dei testi sia come numero uniforme sia come coppia
orizzontale/verticale. Un errore di tipo nei campi JSON opzionali raggiunge il
confine di errore del renderer, senza provocare `std::terminate` nel parser.

Gli script delle proprieta vengono valutati dopo la creazione dei livelli e
usano il namespace del modulo QuickJS. Sono disponibili `applyUserProperties`
all'avvio, `init`, `update`, eventi del cursore sui rettangoli trasformati dei
livelli immagine, `input.cursorWorldPosition`, scrittura delle proprieta dei
livelli e `getEffect(nome).visible`. Gli angoli esposti a SceneScript sono in
gradi; il renderer li converte nei radianti interni. I livelli nascosti usati
come dipendenze continuano a produrre la texture campionata dagli effetti.

Su Hyprland la posizione globale viene letta anche quando una finestra copre il
desktop. Nel panorama viene convertita attraverso la calibrazione dei monitor.
L'opzione **Mouse** deve essere attiva nelle impostazioni di riproduzione;
parallasse e particelle possono restare disattivate. I clic richiedono che la
superficie wallpaper riceva effettivamente il pulsante: non vengono intercettati
i clic destinati alle altre applicazioni.

Verificato il progetto Workshop `3257420414`, con due video, maschera di clipping
e movimento della lente. La conversione dello shader riguarda l'uso di
coordinate larghe nelle espressioni UV a due componenti; non e un emulatore HLSL
generale. La scena predefinita dell'autore contiene un esempio 16:9 in un canvas
5120x1440: nel panorama possono restare zone nere ai lati. Le opzioni per
sostituire i materiali con file personali non sono esposte dalla UI di Lab.
Il payload degli eventi e altre API avanzate di SceneScript restano parziali.

Test sintetico senza asset Workshop, che apre un renderer isolato sul monitor
scelto, muove brevemente il cursore e ne ripristina la posizione:

```sh
python tests/scenescript_integration.py --desktop \
  --app ~/.local/share/wallpaper-lab/runtime/wallpaper-lab \
  --assets ~/.local/share/Steam/steamapps/common/wallpaper_engine/assets \
  --screen DP-6 --output local/scenescript-tests
```

Gli screenshot di un monitor completamente coperto da un gioco possono non
aggiornarsi: per la verifica grafica rendere visibile il desktop. Questo limite
non viene aggirato modificando le finestre o le impostazioni del compositor.

## Proprieta dei creator, zoom e mouse sui monitor

Il pannello destro legge `general.properties` da `project.json`: checkbox,
slider con limiti e passo, combo con etichetta e valore originale, colori RGB e
campi testo. Le immagini HTML delle descrizioni non vengono scaricate o eseguite.
L'ordine segue quello del creator. Le condizioni semplici `nome.value == valore`,
`!=`, `<`, `>`, `<=`, `>=` sono interpretate senza eseguire JavaScript; condizioni
piu complesse restano visibili. I controlli file/directory/scenetexture sono
segnalati come non disponibili, senza simulare una scelta di materiali.

I valori vengono validati e salvati con backup nel profilo del progetto,
separati dai file Workshop. Una modifica ricarica la scena tramite il controller
che conserva il renderer precedente in caso di avvio fallito. Non e ancora un
aggiornamento in tempo reale delle proprieta del motore. I colori usano tre
numeri RGB tra 0 e 1; i campi testo si confermano con Invio.

`wallpaper-lab --properties ID` elenca schema e valori effettivi;
`wallpaper-lab --properties ID CHIAVE VALORE_JSON` salva un override, applicato
al prossimo caricamento. Il pulsante di ripristino cancella gli override.
Le opzioni di formato del creator compaiono fra queste proprieta: non sono
interpretate come immagini alternative o risoluzioni native aggiuntive.

La posizione ora usa il margine del ritaglio, invece di traslare il canvas e
scoprire aree nere. Zoom da 100% a 300% crea ulteriore margine; senza ritaglio una
posizione non ha spazio per muoversi. Restano visibili eventuali aree vuote gia
presenti nella scena dell'autore. Il mouse segue lo stesso ritaglio e zoom.
Su Wayland singolo/duplicato usano l'origine logica effettiva di ciascun monitor,
correggendo il puntatore sul display che non si trova a (0,0).

Le impostazioni grafiche generali restano quelle gia implementate (FPS, audio,
mouse, parallasse e particelle). Questo pannello non aggiunge supporto implicito
agli effetti o alle API del motore ancora mancanti.

## Lingue, temi, font e refresh (0.10.0)

La barra principale contiene Libreria e Carousel con icone e un solo accesso alle
impostazioni, tramite rotella accanto alle tab. Il filtro e a sinistra della
ricerca; luna e aggiornamento sono a destra. Anteprima e Shortcut nel Carousel
hanno larghezza adattata al testo. Shortcut registra direttamente nella barra,
con stato verde e combinazione corrente; la stessa combinazione compare nelle
impostazioni, senza aprire un secondo pannello di registrazione.

Nelle impostazioni, il menu a discesa Lingua offre italiano, inglese, tedesco, spagnolo, francese,
portoghese, olandese, polacco, rumeno e svedese. Il cambio e immediato. Titoli,
testi e opzioni originali degli autori Workshop restano dati del progetto;
le etichette generiche dell'app e i nomi di sistema riconosciuti sono localizzati.
Il catalogo UTF-8 e in `src/translations.json`, compilato nel binario: il generatore
controlla che ogni messaggio abbia nove traduzioni oltre all'italiano e mantenga
i parametri dei formati. Non serve una connessione per cambiare lingua.

Aspetto contiene Standard, Neon ciano, Glass, Neon rosa, Pastello, Viola
profondo, LED, RGB, Oceano e Ambra. I preset cambiano superfici, contrasto,
arrotondamenti e accenti; Neon/LED/RGB includono effetti grafici. Glass usa
superfici traslucide sul fondale morbido dell'app. Pastello rimane opaco.
Comodo, Compatto e Galleria cambiano la densita della griglia mantenendo il
limite minimo delle miniature. Ogni tema ha una variante chiara e una scura:
la luna cambia variante mantenendo la famiglia. Le vecchie preferenze sono migrate
in memoria e salvate al successivo cambio.

Il selettore font mostra ogni nome nel font corrispondente, caricato su richiesta
e mantenuto in cache. Elenca le famiglie installate che supportano l'inglese,
escludendo raccolte di sole icone, emoji e simboli matematici. La scelta modifica
testo, pulsanti, titolo e popup carousel senza riavviare l'app. Le raccolte TTC
sono caricate con FreeType; viene conservato il fallback per i titoli multilingue.
Predefinito ripristina la combinazione originale di font. Nessun font viene
scaricato o incluso nel repository.

L'icona aggiorna rilegge in background le cartelle Workshop delle librerie Steam
rilevate. I thumbnail worker vengono fermati e riavviati prima di sostituire il
catalogo e le texture; selezione e preferiti seguono il percorso del progetto,
cosi restano corretti anche quando cambia l'ordine. Il renderer attivo continua.
Questa funzione rileva i progetti gia scaricati; il browser Workshop in app e una
funzione successiva. Le preferenze UI sono salvate in `settings.json` con chiavi
`language`, `ui_theme`, `light_theme`, `theme_variants`, `layout_density` e `font_family`.

Per diagnostica: `--ui-preferences` mostra preferenze e famiglie disponibili;
`--ui-option CHIAVE NUMERO` salva una preferenza UI (`font` usa l'indice della
famiglia). La build richiede Python per generare il catalogo e Fontconfig per
rilevare le famiglie. La traduzione non richiede Python nel runtime installato.
L'installer accetta `WALLPAPER_LAB_APP_BUILD` per usare una directory di build
applicativa separata; il valore predefinito rimane `build/`.

## Prestazioni e preset Workshop (0.10.0)

I controlli del creator usano un ordine delle proprieta calcolato alla scansione,
lo schema per riferimento e i profili gia caricati. Il disegno non legge il file
profili per ogni controllo. Griglia, filtri, impostazioni e inspector interpolano
lo scroll con un tempo indipendente dal framerate; la griglia evita una seconda
interpolazione sullo spostamento verticale.

I preset con dipendenza Workshop installata ereditano progetto e schema del
wallpaper originale, applicando i valori del preset. Risorse e configurazione
vengono composte nella cache XDG di stato `resolved-projects/`, usando hard link
(o copie su filesystem diversi); i download Steam non vengono modificati. Ogni
preset conserva identita, preferiti e profilo propri. Le dipendenze mancanti e i
cicli producono un errore esplicito. I file locali dei preset web ricevono percorsi
assoluti per i wallpaper che usano URL `file:///`. Le pagine web fisiche sono
caricate come file locali; il custom scheme resta disponibile per gli asset in
contenitori.

Il renderer web applica override tipizzati e aggiorna la texture esistente nelle
regioni indicate da Chromium, evitando una riallocazione completa a ogni paint.
I vecchi campi oggetto nulli non sono piu interpretati come particelle; le caselle
di testo senza valore iniziale partono vuote. Le scene 3D prospettiche restano
escluse con una motivazione specifica: il motore non renderizza ancora i modelli 3D.

`--benchmark-properties ID` misura il costo dei getter dell'inspector senza
modificare il desktop. `tests/catalog_audit.py --desktop --app PERCORSO --output
local/audit` esegue avvii sequenziali in singolo, duplicato e panorama con stato
isolato e lascia il renderer personale in esecuzione. La prova verifica avvio e
breve sopravvivenza, non la fedelta completa degli effetti di ogni progetto.

## Controlli piu intuitivi e audio delle scene (0.11.0)

L'icona monitor accanto a luna e refresh apre un popup dedicato alla disposizione
in Libreria e Carousel. Usa lo stesso editor e lo stesso stato della tab Display:
i salvataggi vengono condivisi. L'inspector non contiene piu Cambia display.
L'editor display e le impostazioni hanno spaziature ridotte.

I colori hanno campione HEX, piano saturazione/luminosita, barra della tonalita e
canali R/G/B da 0 a 255. Il formato normalizzato del creator viene conservato solo
nei dati salvati. Gli slider con intervallo 0–1 mostrano una percentuale.
Lo zoom usa uno slider 100–300%, aggiorna il desktop durante il trascinamento e
salva al rilascio. Il reset delle proprieta creator e rosso e segue tutte le
opzioni dello sfondo; conserva zoom e posizionamento.

I font dell'interfaccia vengono rasterizzati con FreeType, rispettando l'indice
della faccia Fontconfig, incluse raccolte TTC e istanze dei font variabili.
Le anteprime usano un peso leggibile, nome e campione a 18 px nella propria
famiglia. L'elenco evita varianti di copertura Nerd Font/CJK quasi identiche;
una famiglia precedentemente selezionata resta disponibile.

Le scene rispettano startsilent, volume (anche collegato alle proprieta) e i
comandi SceneScript play/pause/stop/isPlaying sui livelli audio. getChildren e
name permettono ai player di scegliere una traccia. Una lista di clip non viene
piu miscelata in simultanea. La decodifica usa stato separato per stream e una
coda non bloccante per SDL; pausa conserva la posizione, stop la azzera, loop e
fine naturale sono distinti. Le risorse audio vengono chiuse prima di SDL.
I livelli di testo eseguono lo script una volta sola. Il bridge espone anche
Vec2/3/4 e le animazioni delle texture usate dalle icone del player.

Wayland usa pacing per output senza attesa EGL del monitor coperto e una coda
eventi con attesa limitata; Chromium continua a elaborare caricamenti anche senza
frame callback. Un monitor coperto non deve bloccare il monitor visibile.

Le prove audio possono scegliere un'uscita SDL esplicita con
WALLPAPER_LAB_AUDIO_OUTPUT e forzare pulseaudio nel solo ambiente del test. Non
cambiano il dispositivo audio predefinito dell'utente. I file delle canzoni usate
per le prove restano in local/ e non vengono distribuiti.


## Workshop Steam integrato e UI compatta (0.12.0)

La nuova tab **Workshop** usa le API Steamworks dello stesso account Steam di
Wallpaper Engine (`431960`). Steam deve essere aperto e l'account deve possedere
Wallpaper Engine. Non servono chiavi Web API, password o cookie nell'app.
La UI rimane nel singolo eseguibile `wallpaper-lab`: il collegamento a
`libsteam_api.so` è dinamico, con ISteamUGC020 e callback a dispatch manuale su
un worker dedicato. Le anteprime statiche HTTPS del CDN Steam sono scaricate
in background (quattro connessioni, trasferimenti interrompibili); le scene
remote non vengono eseguite nel browser Workshop.

Ricerca per testo, ID o link Workshop; autore tramite nome già incontrato nel
catalogo, vanity URL, link profilo o SteamID64. I nomi suggeriti provengono dagli
autori dei risultati. **Steam non espone la ricerca globale di tutti i nomi
utente attraverso UGC** e la ricerca Community anonima risponde HTTP 401: in
quel caso l'interfaccia chiede un profilo/SteamID, senza scegliere un omonimo.
Il pulsante “Altri sfondi di questo autore” usa sempre l'identità esatta.
Testo e tag insieme all'autore vengono applicati su tutte le sue pagine UGC,
entro il limite Steam di 1000 pagine; non solo sui primi cinquanta risultati.

I tag ufficiali del Workshop sono divisi per tipo, caratteristiche (audio,
media, personalizzazione, ecc.), genere, risoluzioni e classificazione. La scelta
**Tutti i tag (AND)** richiede ogni tag; **Almeno un tag (OR)** ne richiede uno.
Nome e autore restringono sempre la ricerca. Ordinamento e pagine da 50
risultati usano Steam. La scheda mostra autore, descrizione, tag, dimensione
quando dichiarata, date, voti, sottoscrizioni, dipendenze e link originali.

**Download** sottoscrive l'elemento e avvia il trasferimento Steam a priorità
normale, senza mettere in pausa altri download. I preset scaricano anche le
dipendenze richieste, con controllo di app ID e cicli/limiti. Avanzamento e
possibilità di riprovare restano nella scheda. Il completamento richiede il
callback Steam, lo stato installato e `project.json`; l'importazione aggiorna
automaticamente la libreria. Per gli asset di dipendenza non viene richiesto
`project.json`. Un download non amplia automaticamente la compatibilità del
renderer: valgono i limiti documentati per scene 3D o applicazioni Windows.

L'installer locale riutilizza il redistributable Steamworks già presente nel
runtime precedente o nell'installazione WallpaperX. `WALLPAPER_LAB_STEAM_API`
permette un percorso esplicito. Le chiamate di test sono disponibili come
`--workshop-search '{"text":"Frieren","tags":["Scene"],"any":false}'` e
`--workshop-download ID` (quest'ultimo esegue una vera sottoscrizione/download).
La libreria HTTP di build è `libcurl`, scoperta tramite pkg-config.

**Ctrl+L** porta alla ricerca. L'editor UTF-8 comune supporta Ctrl+A/C/X/V,
cursore, Home/End e selezione da tastiera; vale anche per i campi di testo del
creator. Il pannello filtri si ridimensiona trascinando il bordo, con larghezza
persistente. Nella libreria “Tutti i criteri (AND)” combina tipo, audio e
risoluzioni; “Almeno un criterio (OR)” accetta uno qualunque dei criteri attivi.
Nessun filtro selezionato mostra tutto.

Le impostazioni sono una finestra centrata di massimo 860 × 650, divisa in
Generali, Display, Riproduzione, Carousel, Aspetto e Workshop. Le scelte usano
menu a tendina con liste limitate e scroll. L'editor Display conserva un solo
comando di salvataggio, condiviso con il popup monitor. Sono disponibili peso
del testo (medio/semigrassetto/grassetto), riduzione delle animazioni e politica
di pausa per applicazioni a schermo intero, oltre ai controlli esistenti.
I font leggeri senza una faccia più pesante ricevono il peso mancante; le
coordinate dei font variabili vengono rispettate. I glifi mancanti usano il
fallback CJK e una cache di indici evita ricerche lineari ripetute per il testo.

“**Colore principale del wallpaper**” chiarisce l'ex “Theme color”: è la
proprietà creator `schemecolor`, non il tema dell'interfaccia. Cambia il rendering
solo se collegata a un effetto/proprietà/script della scena. Non viene applicata
una tinta globale artificiale ai progetti che non la usano.

L'eliminazione dello sfondo attivo prepara prima il primo altro preferito
compatibile, oppure il primo sfondo disponibile in libreria. Un avvio fallito
conserva processo e file originali. Se non resta alcuno sfondo, viene fermato
solo il renderer dell'app identificato da PID, start time ed eseguibile. I file
vanno nel cestino recuperabile; se il cestino fallisce sull'ultimo wallpaper,
l'app tenta di ripristinarlo. Le sottoscrizioni Steam restano esplicite: Steam
può riscaricare contenuti ancora sottoscritti, come indica la conferma.

Prove opt-in (le ultime due richiedono sessione grafica; le opzioni download
eseguono vere sottoscrizioni):

```sh
python3 tests/workshop_integration.py --app local/ui12-runtime/wallpaper-lab --output local/steam-test
python3 tests/workshop_ui_integration.py --app local/ui12-runtime/wallpaper-lab --output local/workshop-ui-test
python3 tests/active_delete_integration.py --app local/ui12-runtime/wallpaper-lab --output local/delete-test
```

## Versione 0.13 — monitor indipendenti e menu sovrapposti

La documentazione ufficiale aggiornata è in [Guida utente](docs/USER_GUIDE.md),
[Architettura](docs/ARCHITECTURE.md) e [Pubblicazione](docs/RELEASING.md).

- Filtri Workshop automatici con debounce di 450 ms e reset completo.
- Dropdown a overlay, multiselezione per i filtri e risoluzioni senza sigle HD/p.
- Una sola luna, accanto al tema in Aspetto.
- Renderer indipendente per ogni monitor singolo, assegnazioni persistenti e restore multiplo.
- Otto transizioni oltre al taglio diretto: fade, nero, zoom, slide, wipe e iris
  verso il mouse, con anteprima separata da quella dello switcher.
- Metadati MPRIS iniziali, costanti MediaPlaybackEvent e timeline convertite in secondi.
- Shaping e fallback Unicode Pango/Cairo nella UI e nei testi Unicode delle scene.
- 22 lingue selezionabili: le nuove 12 hanno i menu principali tradotti e fallback
  inglese per i messaggi avanzati; localizzazione completa ancora da revisionare.

Dipendenza aggiuntiva di build e runtime: `pangocairo` tramite pkg-config.
Le modifiche al motore sono salvate in `engine-transitions-media-text.patch`.
La repository resta privata; da questa versione codice e documentazione vengono
sincronizzati su GitHub per ogni modifica completata, secondo richiesta di Simone.
