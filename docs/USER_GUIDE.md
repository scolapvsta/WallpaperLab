# Wallpaper Lab — guida 0.14

## Libreria e monitor

Clicca una card per applicarla. In Display scegli panorama, duplicazione oppure
un monitor. In modalità singola seleziona il monitor nella mappa, poi clicca lo
sfondo: gli altri monitor mantengono il proprio renderer e wallpaper. Le
assegnazioni vengono salvate per nome dell'uscita e ripristinate insieme.

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

Le transizioni campionano solo il framebuffer del wallpaper precedente, a
risoluzione nativa: non catturano finestre, notifiche o applicazioni. Se il
renderer precedente non fornisce un fotogramma (ad esempio è sospeso), si applica
lo sfondo senza transizione. Le immagini temporanee rimangono nello stato XDG locale.

## Aspetto e lingue

La luna è accanto al tema nelle impostazioni Aspetto, all'interno della stessa
larghezza degli altri campi. Tutti i temi hanno una variante chiara e una scura.
Le palette di riferimento sono Catppuccin (pastello), Dracula (neon) e Rosé Pine
(toni vetro/oceano/ambra). RGB mantiene un accento animato, disattivabile con
Riduci animazioni. La trasparenza è indipendente: il cursore Opacità della finestra
varia dal 70 al 100%, con 100% opaco; non modifica i renderer desktop.

Le 22 lingue comprendono italiano, inglese, tedesco, spagnolo, francese, portoghese,
olandese, polacco, romeno, svedese, cinese semplificato/tradizionale, giapponese,
coreano, hindi, russo, arabo, bengalese, indonesiano, turco, vietnamita e urdu.
Il catalogo contiene 305 messaggi completi, comprese le impostazioni avanzate
che prima ricadevano sull'inglese. Non sostituisce una revisione madrelingua;
i contenuti degli autori e le categorie/tag forniti da Steam restano originali.

Pango/Cairo gestisce shaping, scritture RTL e font di fallback per le scritte
Unicode dell'app. I testi Unicode delle scene usano lo stesso sistema di shaping;
i font necessari devono essere installati nel sistema. Il layout dei pannelli
non viene ancora specchiato nelle lingue RTL.

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
