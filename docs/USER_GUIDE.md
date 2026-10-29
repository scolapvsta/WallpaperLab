# Wallpaper Lab — guida 0.15

## Libreria e monitor

Clicca una card per applicarla. In Display scegli panorama, duplicazione oppure
un monitor. In modalità singola seleziona il monitor nella mappa, poi clicca lo
sfondo: gli altri monitor mantengono il proprio renderer e wallpaper. Le
assegnazioni vengono salvate per nome dell'uscita e ripristinate insieme.
Il pannello dello sfondo offre anche il campo Monitor: è attivo solo in modalità
singola. Scegliere un'uscita applica a quella lo sfondo selezionato. Cliccare lo
sfondo già attivo sull'uscita selezionata non riavvia il renderer e non anima una
transizione. Le modifiche effettive alle sue proprietà continuano ad applicarsi.

I menu si aprono sopra gli altri campi, con scrollbar trascinabile quando serve.
Le risoluzioni sono l'ultima sezione del pannello, raggruppate per monitor singolo,
doppio, ultrawide, triplo e verticale. La casella di categoria seleziona tutte le
risoluzioni del gruppo: basta che lo sfondo ne supporti una. Le risoluzioni scelte
singolarmente seguono AND (deve supportarle tutte) oppure OR (ne basta una).
Con AND anche gli altri criteri devono corrispondere; con OR basta uno dei criteri
attivi. Nome e autore restringono sempre la ricerca Workshop. Reset cancella le
selezioni. La barra laterale è ridimensionabile trascinandone il bordo.

Nei filtri scegli 50, 100 o 200 risultati per pagina. Il piede della griglia mostra
la pagina attuale e il totale. Ctrl + L sposta il cursore sulla ricerca; Ctrl + A,
C e V selezionano, copiano e incollano. Tieni premuto il tasto centrale e trascina
in verticale per scorrere griglia, proprietà, filtri, impostazioni e menu.

Le sezioni dello sfondo sono richiudibili: Display di destinazione, Immagine e
ritaglio, Riproduzione e Proprietà del creator. Immagine raccoglie regolazioni,
zoom e spostamento; il ripristino delle proprietà del creator rimane in fondo,
separato dai controlli immagine.

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
In Griglia l'evidenziazione anima il contorno senza ingrandire le immagini oltre
il mosaico; contorno delle celle e bordo esterno coincidono.

Le transizioni campionano solo il framebuffer del wallpaper precedente, a
risoluzione nativa: non catturano finestre, notifiche o applicazioni. Se il
renderer precedente non fornisce un fotogramma (ad esempio è sospeso), si applica
lo sfondo senza transizione. Le immagini temporanee rimangono nello stato XDG locale.

## Aspetto e lingue

Aspetto separa le collezioni Temi scuri e Temi chiari: ogni nome è un tema
indipendente, senza un interruttore chiaro/scuro che ne alteri la palette.

- Scuri: Charcoal, Cyber Neon, Aurora Glass, Sakura Neon, Midnight Violet,
  Matrix LED, Prism RGB, Nord, Rosé Pine, Catppuccin Mocha e Comfy Mint.
- Chiari: Paper, Catppuccin Latte, Rosé Dawn, Sakura Paper e Frost.

Cambiano superfici, accenti, contrasto e arrotondamenti. I neon aggiungono luce
e contorni; Matrix LED usa una trama puntinata e dettagli verdi, Prism RGB un
bordo cromatico, Aurora Glass riflessi e un fondale morbido; Sakura ha particelle
discrete. Riduci animazioni ferma gli effetti in movimento. Sono adattamenti
propri ispirati anche a Midnight/System24 per Discord, Comfy per Spicetify,
Catppuccin e Rosé Pine; non port ufficiali.

La Trasparenza della UI è indipendente dal tema: 0% è opaco, 30% è il massimo.
Il cursore occupa la stessa colonna degli altri campi. Varia soltanto le superfici
dell'interfaccia; miniature, immagini Workshop e scene live restano opache,
così come il testo. Il renderer desktop non viene alterato. Le preferenze
precedenti vengono migrate al tema indipendente più vicino.

Le 22 lingue comprendono italiano, inglese, tedesco, spagnolo, francese, portoghese,
olandese, polacco, romeno, svedese, cinese semplificato/tradizionale, giapponese,
coreano, hindi, russo, arabo, bengalese, indonesiano, turco, vietnamita e urdu.
Il catalogo contiene 328 messaggi completi, comprese le impostazioni avanzate
che prima ricadevano sull'inglese. Non sostituisce una revisione madrelingua;
i contenuti degli autori e le categorie/tag forniti da Steam restano originali.

Pango/Cairo gestisce shaping, scritture RTL e font di fallback per le scritte
Unicode dell'app. I testi Unicode delle scene usano lo stesso sistema di shaping;
i font necessari devono essere installati nel sistema. Il layout dei pannelli
non viene ancora specchiato nelle lingue RTL.

## Integrazione Caelestia

Impostazioni → Integrazioni → Caelestia → Colori dal wallpaper è disattivata
inizialmente. Richiede il comando `caelestia` e la shell Caelestia che legge
`$XDG_STATE_HOME/caelestia/scheme.json`. Quando è attiva, applicare uno sfondo
genera una palette dalla sua anteprima e aggiorna barre e widget. L'operazione
non cambia il wallpaper tramite Caelestia e non ricolora altre applicazioni.

La palette è globale: con più monitor conta l'ultimo wallpaper applicato al
monitor selezionato. Usa i colori dell'anteprima, non ogni fotogramma animato;
le anteprime di prova non cambiano la palette. La generazione avviene fuori dal
thread grafico e un errore dell'integrazione non interrompe il wallpaper.
Disattivando l'opzione viene ripristinata la palette precedente, se quella attuale
è ancora l'ultima pubblicata dall'app. Le modifiche manuali successive vengono
rispettate. Non vengono avviate automaticamente shell o servizi mancanti.

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
Ripristina immagine rimette i tre valori neutri, senza cambiare proprietà del
creator, posizione o riproduzione. Le regolazioni sono un passaggio finale sul
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
