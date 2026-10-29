# Wallpaper Lab — guida 0.19

## Libreria e monitor

Clicca una card per applicarla. In Display scegli panorama, duplicazione oppure
un monitor. In modalità singola seleziona il monitor nella mappa, poi clicca lo
sfondo: gli altri monitor mantengono il proprio renderer e wallpaper. Le
assegnazioni vengono salvate per nome dell'uscita e ripristinate insieme.
In modalità Un monitor, clicca il monitor nella mappa Display: quello evidenziato
resta la destinazione anche dopo aver chiuso il popup con X o Esc. Il prossimo
wallpaper cliccato viene applicato a quell'uscita. Il pannello dello sfondo mostra
la destinazione senza aggiungere un secondo selettore. In panorama/duplicato il
target è il gruppo dei display. Cliccare il wallpaper già attivo non riavvia il
renderer e non anima una transizione; le modifiche alle proprietà restano effettive.

Le scelte con due sole opzioni usano un toggle: impostazioni generali e locali,
shortcut, inversione dello switcher, contenuti maturi, integrazione e proprietà
del creator. Il colore del tema indica lo stato attivo. Per alternative come
AND/OR, misurazione e risoluzione viene mostrato anche il nome della scelta
corrente. Con più di due opzioni resta il menu; le selezioni multiple mantengono
la possibilità di combinare più valori.

I menu si aprono sopra gli altri campi, con scrollbar trascinabile quando serve.
Le risoluzioni sono l'ultima sezione del pannello, raggruppate per monitor singolo,
doppio, ultrawide, triplo e verticale. La casella di categoria seleziona tutte le
risoluzioni del gruppo: basta che lo sfondo ne supporti una. Le risoluzioni scelte
singolarmente seguono AND (deve supportarle tutte) oppure OR (ne basta una).
Con AND anche gli altri criteri devono corrispondere; con OR basta uno dei criteri
attivi. Nome e autore restringono sempre la ricerca Workshop. Reset cancella le
selezioni. La barra laterale è ridimensionabile trascinandone il bordo.

Nel piede della griglia scegli 50, 100 o 200 risultati per pagina, accanto alla
pagina attuale e al totale. Le frecce navigano le pagine. Lo stesso controllo è
presente nel Workshop; la dimensione pagina non è un criterio di filtro. Ctrl + L sposta il cursore sulla ricerca; Ctrl + A,
C e V selezionano, copiano e incollano. Tieni premuto il tasto centrale e trascina
in verticale per scorrere griglia, proprietà, filtri, impostazioni e menu.

Le sezioni dello sfondo sono richiudibili: Display di destinazione, Immagine e
ritaglio, Riproduzione e Proprietà del creator. Immagine raccoglie regolazioni,
zoom e spostamento; il ripristino delle proprietà del creator rimane in fondo,
separato dai controlli immagine.

Specchia orizzontalmente e Specchia verticalmente sono interruttori: il colore
di accento indica che lo specchio è attivo. Le modifiche restano nel profilo di
quel wallpaper e si vedono senza riavviare la scena.

Il pulsante rosso **Ripristina immagine** riporta a neutro luminosità, contrasto,
saturazione, temperatura, tonalità, gamma, vignettatura, rotazione e specchiature.
Non cambia posizione, zoom, riproduzione o proprietà del creator. Salva subito
il ripristino per il solo wallpaper selezionato. Anche il ripristino del creator
è rosso; importazione e download usano invece l'accento del tema.

## File locali e cartelle

L'icona cartella con + importa immagini o video; puoi anche trascinarli nella
finestra. La selezione nativa richiede Zenity oppure KDialog. L'importazione copia
i file: gli originali rimangono al loro posto. Nome, risoluzione e miniatura
vengono ricavati automaticamente con ffprobe/ffmpeg. Reimportare lo stesso file,
con percorso, dimensione e data invariati, riutilizza la copia già importata.

Formati accettati: PNG, JPEG, WebP, GIF, BMP, TIFF, AVIF e video MP4, WebM, MKV,
MOV, AVI, M4V; il codec deve essere supportato dal renderer e da FFmpeg installati.
Le immagini usano un progetto web gestito dall'app; i video usano il renderer
video. Non è richiesta la cartella asset di Wallpaper Engine per questi media.
Non vengono importati eseguibili o progetti arbitrari tramite il selettore file.

Le card riportano Steam o Locale. Il filtro Origine si combina con gli altri
criteri secondo AND/OR. In Impostazioni → Librerie puoi scegliere la cartella
Workshop Steam (normalmente `steamapps/workshop/content/431960`) e quella locale.
La cartella locale iniziale viene creata in
`$XDG_DATA_HOME/wallpaper-lab/wallpapers`, oppure
`~/.local/share/wallpaper-lab/wallpapers`. Cambiare percorso cambia la cartella
letta e la destinazione delle importazioni successive: non sposta i file esistenti.
Il percorso Workshop serve alla scansione; la destinazione dei download resta
gestita dal client Steam e dalle sue librerie.

Da terminale: `wallpaper-lab --import FILE...` e
`wallpaper-lab --library-path local|steam CARTELLA`.

## Workshop

È necessario Steam aperto, con un account che possiede Wallpaper Engine. Il
catalogo e i download usano quel client, senza password o cookie nell'app.
Modifiche ai filtri, all'ordinamento, all'autore e al testo avviano la ricerca
automaticamente dopo una breve pausa. Reset cancella anche autore, testo e ordine.
Puoi scaricare anche dal pulsante sulla miniatura, senza aprire i dettagli.
Il download sottoscrive l'elemento e le sue dipendenze tramite Steam; la libreria
si aggiorna al termine. La compatibilità del renderer non è garantita dal download.

## Carousel

Il primo menu sceglie lo stile dello switcher; l'occhio adiacente ne mostra
l'anteprima. Il secondo sceglie la transizione desktop e ha una propria anteprima:
dissolvenza, fade attraverso nero, zoom, slide, wipe, iris e chiusura/apertura
verso il mouse. La durata iniziale è 700 ms. Nell'anteprima clicca nell'immagine
per cambiare l'origine dell'iris. Il desktop usa il cursore del monitor interessato.
Entrambe le anteprime si chiudono con Esc o clic esterno. La prima pressione dello
shortcut apre lo switcher sullo sfondo attivo del monitor selezionato; quelle
successive seguono l'ordine dei preferiti nell'app. Se lo sfondo attivo non è tra
i preferiti, compare come punto di partenza temporaneo e poi si passa al primo
preferito, senza modificare la lista. Confermarlo nuovamente non cambia il desktop.
In Griglia le immagini tornano a ingrandirsi sopra il mosaico, conservando angoli
arrotondati e bordi; lo spazio trasparente esterno ospita lo zoom. Il mouse evidenzia
la cella puntata e un clic la conferma. Negli altri layout il clic può confermare
una card; il solo passaggio del mouse non sposta la striscia sotto il puntatore.
Il wallpaper in uso ha un piccolo pallino bianco in tutti i layout e nelle card,
senza la scritta ATTIVO. Il popup resta aperto durante la navigazione con il mouse.

L'occhio dello switcher apre ora un popup dentro l'app, come quello della
transizione, con X, Esc e clic esterno. Le miniature si animano con lo stesso
codice del selettore reale; la demo non cambia il desktop.

La bacchetta accanto a ciascun selettore apre un pannello compatto di personalizzazione:

- Switcher: dimensione 70–140%, zoom 105–150%, entrata/uscita 60–600 ms,
  movimento 100–900 ms. Nei layout a striscia è disponibile l'inversione; Onda
  aggiunge velocità 0–300% e ampiezza 0–40 px.
- Transizioni: durata 100–2500 ms e curva morbida, lineare o accelerazione/frenata.
  Scorrimento e Tendina offrono quattro direzioni; gli zoom hanno intensità 5–60%.

I controlli si salvano al rilascio o alla scelta e sono condivisi dalla tab
Carousel e dalle impostazioni principali. Riduci animazioni continua ad avere
precedenza sul selettore reale e sulle transizioni desktop.

Le transizioni campionano solo il framebuffer del wallpaper precedente, a
risoluzione nativa: non catturano finestre, notifiche o applicazioni. Se il
renderer precedente non fornisce un fotogramma (ad esempio è sospeso), si applica
lo sfondo senza transizione. Le immagini temporanee rimangono nello stato XDG locale.

## Aspetto e lingue

**Themes** propone Default, Strawberry core, 2077 e Pixel Theme. Default ha tre
palette sobrie, Strawberry tre pastello, 2077 undici neon e Pixel quattro palette
da gioco. Ogni tema ricorda palette e background. La modalità è stabilita dal
tema: Strawberry chiaro, gli altri scuri. Le nuove installazioni partono da Default.

**Suggested Theme Fonts** sceglie il font del tema. **General Fonts**, subito sotto,
è un override facoltativo: soltanto con la spunta attiva applica la famiglia
selezionata a tutta la UI e allo switcher. La spunta e il menu rientrano nella
stessa larghezza degli altri campi. Spegnendo la spunta torna il font del tema.

Quattro background inclusi per ogni tema, con una preview vicina, più **Carica dal
PC**. **Ripristina tema** torna al primo fondo incluso. Strawberry ha composizioni
più diverse, Pixel sprite originali e scenari da gioco. Riduci animazioni ferma gli
effetti decorativi. I quattro stili dello switcher usano ora cornici, colori e
dettagli del tema selezionato, anche nel popup reale.
[Dettagli e limiti](THEMES.md).

La Trasparenza della UI è indipendente dal tema: 0% è opaco, 30% è il massimo.
Il cursore occupa la stessa colonna degli altri campi. Varia soltanto le superfici
dell'interfaccia; miniature, immagini Workshop e scene live restano opache,
così come il testo. Il renderer desktop non viene alterato. Le preferenze
precedenti Strawberry mantengono la palette; gli altri preset passano a Strawberry Milk,
conservando modalità e font.

Le 22 lingue comprendono italiano, inglese, tedesco, spagnolo, francese, portoghese,
olandese, polacco, romeno, svedese, cinese semplificato/tradizionale, giapponese,
coreano, hindi, russo, arabo, bengalese, indonesiano, turco, vietnamita e urdu.
Il catalogo contiene 367 chiavi; le nuove opzioni avanzate della 0.16 sono in
italiano/inglese, con testi inglesi nelle altre lingue dove manca una revisione. Non sostituisce una revisione madrelingua;
i contenuti degli autori e le categorie/tag forniti da Steam restano originali.

Pango/Cairo gestisce shaping, scritture RTL e font di fallback per le scritte
Unicode dell'app. I testi Unicode delle scene usano lo stesso sistema di shaping;
i font necessari devono essere installati nel sistema. Il layout dei pannelli
non viene ancora specchiato nelle lingue RTL.

## Integrazione Caelestia

Impostazioni → Integrazioni → Caelestia → Colori dal wallpaper resta un extra
facoltativo, inizialmente disattivato sulle nuove installazioni. La shell Caelestia
va installata separatamente nel desktop. Wallpaper Lab non installa Hyprland,
non sostituisce il desktop e non avvia servizi. Il generatore colore e i preset
sono inclusi nel runtime, ma le dipendenze Python vengono preparate soltanto
all'attivazione: usa i moduli già disponibili oppure un venv privato in
`$XDG_DATA_HOME/wallpaper-lab/integrations/caelestia/venv`. Può essere necessaria
una connessione Internet e il modulo venv del Python di sistema; niente sudo o
modifiche ai pacchetti Python globali.

Quattro preset dinamici — Vivace, Toni morbidi, Espressiva, Fedele al wallpaper —
generano colori Material You dal wallpaper, con luminosità automatica, scura o
chiara. Vivace è il default e non eredita una variante monocromatica dallo schema
Caelestia precedente. Le superfici ricevono una tinta leggera per rendere visibile
la tonalità anche su barre e widget. L'app pubblica una palette compatibile con
lo schema `dynamic/default` esistente; non modifica l'elenco degli schemi fissi
installati dal pacchetto Caelestia.

Viene campionato il fotogramma effettivo del renderer, con crop e regolazioni;
se non è disponibile (per esempio in pausa), viene usata l'anteprima del creator.
La palette si aggiorna al cambio wallpaper e al salvataggio delle regolazioni,
non a ogni frame o battito musicale. È globale: con più monitor conta l'ultimo
wallpaper applicato al target selezionato. La pubblicazione riguarda
`$XDG_STATE_HOME/caelestia/scheme.json`, senza cambiare il wallpaper statico
Caelestia o i temi delle altre applicazioni. Il browser wallpaper di Caelestia
continua a mostrare la propria cartella di immagini statiche.

Disattivando l'opzione viene ripristinata la palette precedente se quella attuale
è ancora l'ultima pubblicata dall'app. Le modifiche manuali successive vengono
rispettate. Generazione e installazione opzionale avvengono fuori dal thread UI;
risultati tardivi dopo disable/cambio wallpaper vengono scartati. Eventuali errori
appaiono nel pannello Integrazioni e non interrompono il renderer.

## Media

Il renderer riceve titolo, artista, album, stato e timeline via MPRIS (Spotify,
browser o altri player che lo espongono). Le timeline sono convertite in secondi
e inviate solo quando la durata è disponibile. I metadati già presenti vengono
inviati anche agli script appena inizializzati. Non viene importata la libreria
personale dei preferiti Spotify: MPRIS non espone quell'elenco. Le playlist audio
incluse dal creator nel wallpaper restano distinte dai metadati del player di sistema.

## Immagine e riproduzione individuale

Nell'inspector, Immagine offre luminosità (−100…+100%), contrasto e saturazione
(0…200%). I cambiamenti si vedono durante il trascinamento e si salvano al rilascio.
Ripristina immagine rimette tutti i valori neutri, senza cambiare proprietà del
creator, zoom/posizione o riproduzione. Temperatura (−100…+100%), tonalità
(−180…+180°), gamma (0,25…3), vignettatura, rotazione (−180…+180°) e specchio
orizzontale/verticale sono salvati nello stesso profilo. La rotazione ritaglia
automaticamente il risultato per riempire lo schermo senza angoli neri. Sono
manipolazioni del fotogramma di ciascuna uscita: in panorama i display vengono
trasformati separatamente. Non ruotano le coordinate logiche degli oggetti
interattivi della scena. Le regolazioni sono un passaggio finale sul
fotogramma: non modificano i file del Workshop e non promettono una gestione HDR.

Le cinque opzioni di Riproduzione per questo sfondo sono salvate separatamente:
audio dello sfondo, reazione alla musica, interazione con il mouse, parallasse e
particelle. Per essere effettiva, ciascuna deve essere attiva anche nelle impostazioni
generali. Per esempio, audio globale acceso + audio locale spento = silenzio;
audio globale spento + audio locale acceso = ancora silenzio. Se un progetto non
implementa una funzione, abilitarla non la aggiunge. Le opzioni di riproduzione
individuali ricaricano lo sfondo attivo; gli slider immagine aggiornano lo stesso
renderer senza riavvio. Le preferenze seguono il progetto quando viene cambiato,
riaperto o richiamato dal carousel, su qualsiasi modalità display.


## Piattaforme

La build corrente è Linux x86_64; l'ambiente verificato è Arch Linux con Hyprland.
Le parti POSIX, il motore Linux, PipeWire/PulseAudio e l'integrazione dei monitor
non sono un port Windows/macOS. Raylib e Material You sono multipiattaforma, ma
questo non rende multipiattaforma l'app completa. Ubuntu e altre distribuzioni
Linux sono obiettivi possibili compilando con le dipendenze adatte, ancora da
verificare; su altri compositor anche shortcut e gestione output richiedono prove.
L'installer attuale installa il runtime già compilato per questo ambiente, con
backup e ripristino dei renderer; non è un installer universale di sistema.

## Transizioni e animazioni del desktop

Gli effetti dal puntatore coprono esattamente la distanza fino all'angolo più
lontano: la durata scelta non include una parte finale senza movimento visibile.
Il bordo dell'iris è antialias. Durata, curva, direzione e intensità restano
personalizzabili dal pulsante accanto alla transizione.

Su Hyprland l'app evita che lo slide/fade automatico delle layer surface si sommi
ai propri effetti. La regola riguarda soltanto `wallpaper-lab-background`, viene
applicata in memoria a ogni avvio del renderer e non cambia animazioni delle
altre finestre né file di configurazione. Le prove grafiche native da sole non
rilevano queste animazioni aggiunte dal compositor: la regressione usa anche
campioni del desktop con un'immagine a fasce, prima e dopo il passaggio.


## Mute immediato, anche per i progetti web — 0.22

**Audio dello sfondo** nelle impostazioni generali e nel pannello del wallpaper
sono due interruttori in serie: basta spegnerne uno per silenziare il wallpaper
attuale. La modifica viene recepita dal renderer senza riavviarlo; gli altri
controlli di riproduzione conservano il comportamento indicato nel pannello.
Il browser applica il mute a elementi audio/video, Web Audio e iframe, anche se
la pagina prova a riattivare il proprio player o viene ricaricata. Il mute non
disabilita la cattura per gli effetti che reagiscono alla musica. La preferenza
locale resta salvata per quel progetto; riattivare solo il controllo globale non
supera un mute locale.
