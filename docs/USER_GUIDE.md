# Wallpaper Lab — guida 0.13

## Libreria e monitor

Clicca una card per applicarla. In Display scegli panorama, duplicazione oppure
un monitor. In modalità singola seleziona il monitor nella mappa, poi clicca lo
sfondo: gli altri monitor mantengono il proprio renderer e wallpaper. Le
assegnazioni vengono salvate per nome dell'uscita e ripristinate insieme.

I filtri usano menu sovrapposti, anche con selezioni multiple; non spostano i
campi sottostanti. Le risoluzioni riportano solo larghezza × altezza. AND richiede
tutti i criteri, OR ne richiede almeno uno. Reset cancella le selezioni.

## Workshop

È necessario Steam aperto, con un account che possiede Wallpaper Engine. Il
catalogo e i download usano quel client, senza password o cookie nell'app.
Modifiche ai filtri, all'ordinamento, all'autore e al testo avviano la ricerca
automaticamente dopo una breve pausa. Reset cancella anche autore, testo e ordine.
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

La luna è accanto al tema nelle impostazioni Aspetto. Le nuove lingue includono
cinese semplificato/tradizionale, giapponese, coreano, hindi, russo, arabo, bengalese,
indonesiano, turco, vietnamita e urdu. I menu principali sono tradotti; nelle nuove
lingue i messaggi avanzati usano ancora l'inglese. Le dieci lingue precedenti
mantengono il catalogo completo. Questa è una localizzazione iniziale, non una
revisione linguistica completa.

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
