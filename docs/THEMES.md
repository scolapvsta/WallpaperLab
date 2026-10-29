# Themes e palette dedicate — 0.21

**Themes** è sempre una dropdown, anche con due sole voci. I temi attivi sono
Strawberry core e **2077**. **Color Palettes** contiene esclusivamente le palette
del tema selezionato; **Mode** cambia Dark/Light. Ciascun tema ricorda la propria
palette e modalità. La scelta del font e del peso resta indipendente dal tema.

Le 29 palette storiche e Originale sono stati rimossi dal catalogo attivo, su
richiesta dell'utente. Il backup locale `local/backups/themes-0.20/` e il commit
0.20 (`78bc1ec`) ne conservano l'implementazione. Le vecchie preferenze Strawberry
Milk/Raspberry Ribbon/Vanilla Shortcake migrano mantenendo la selezione; gli altri
preset passano a Strawberry Milk. La modalità e il font salvati vengono conservati.

## Strawberry core

Tre palette: **Strawberry Milk**, **Raspberry Ribbon**, **Vanilla Shortcake**.
Fondo vichy, fragole, fiocchi e pizzi. Pulsanti a biscotto giallo miele con estremità
arrotondate, incisione interna e piccoli puntini; il testo sui biscotti rimane
marrone per il contrasto anche in modalità scura. Coniglietti interi con orecchio
piegato, occhi lucidi, guance rosa, grande fiocco a quadretti e biscottino in mano.
Il font non viene modificato o reso corsivo dal tema.

## 2077

| Palette | Accento | Atmosfera |
| --- | --- | --- |
| Violet Afterglow | Viola | Grattacieli e foschia notturna |
| Pink Voltage | Fucsia | Insegne rosa acceso |
| Electric Blue | Ciano | Blu elettrico e rosa |
| Night Drive | Rosa | Blu notte e neon rosa/ciano |
| Hazard Yellow | Giallo | Segnaletica gialla e arancio |

Scena urbana originale disegnata in raylib e memorizzata in una texture: skyline
su tre livelli, finestre pixel, insegne geometriche ispirate al katakana,
cartelloni, cavi e prospettiva della strada con riflessi. Pannelli con angoli
smussati a taglio, contorni luminosi, pioggia animata, trama a scansione, LED in
movimento e brevi glitch discreti confinati allo sfondo. Riduci animazioni ferma
pioggia e LED e disabilita i glitch. I contenuti e i testi restano stabili.
La variante Light usa colori derivati dall'accento per mantenere leggibilità.

I riferimenti allegati ispirano l'atmosfera e le forme. Non sono incorporati
come fotografie né copiati come asset; le illustrazioni sono vettoriali originali.

## Sfondo dell'app dal PC

Aspetto → **Sfondo dell’app → Carica dal PC** apre il selettore di immagini.
L'immagine viene decodificata e salvata come PNG entro 2048 × 2048 pixel nei dati
XDG `wallpaper-lab/app-backgrounds/`; il file originale rimane intatto e non è
necessario conservarlo nello stesso percorso. L'importazione è asincrona. Un file
non valido conserva la selezione precedente. **Ripristina tema** torna al fondo
illustrato del tema; le copie precedenti restano nei dati locali.

Il fondo personalizzato è comune ai temi, riempie la finestra con ritaglio centrale
e riceve una leggera tinta del tema. Sostituisce la scena/vichy dello sfondo, mentre
pannelli, tasti e testata mantengono il proprio stile. Non modifica il wallpaper
desktop, le miniature, l'inspector o il catalogo dei progetti. GIF e altri formati
animati sono importati come immagine statica (primo frame). Serve zenity o kdialog
per la scelta e FFmpeg per la conversione, già usati per i media locali.

## Layout e limiti

Le etichette dei controlli usano la stessa misura e lo stesso rendering Pango,
centratura basata sull'altezza effettiva, troncamento UTF-8 e spazio riservato alla
freccia nei menu. La testata usa la stessa misura per disporre testi e icone.
I quattro layout del carousel conservano struttura e animazioni. Il popup
transitorio mantiene la geometria precedente e riceve i colori; i decori completi
sono nella finestra principale e nelle sue anteprime.

# Archivio: collezioni e palette — 0.18

La collezione sceglie la famiglia visiva e filtra il menu Temi. La palette scelta
resta indipendente dalla trasparenza della UI. I preset chiari e scuri hanno nomi
propri e possono convivere nella stessa famiglia.

| Collezione | Palette | Stile |
| --- | ---: | --- |
| Classici | 5 | superfici sobrie, contorni discreti; Charcoal, Paper, Nord, Frost, Plum Noir |
| Neon | 6 | accenti luminosi e bordi con alone; Cyber, Sakura, Violet, Lime Circuit, Arcade Violet, Ember |
| Pastello | 9 | colori smorzati e superfici morbide; Latte/Mocha, Rosé, Sakura Paper, Mint, Lavender, Peach |
| Vetro | 3 | riflessi fini, gradienti e bordi lucidi; Aurora, Moonstone, Sea Glass |
| Pixel | 3 | pannelli squadrati, piccoli dettagli a griglia; Orchid, Ocean, Pocket Sage |
| LED e RGB | 3 | punti LED o bordo cromatico animato; Matrix, Prism, Persimmon |

Pixel cambia il trattamento dei pannelli, senza rendere il testo poco leggibile
né pixelare le anteprime. Riduci animazioni ferma gli effetti decorativi. Vetro
non impone trasparenza: lo slider continua a controllare soltanto la UI.
Gli ID dei 16 preset precedenti restano stabili; la collezione viene ricavata dal
preset scelto, senza sostituire la palette esistente durante l'aggiornamento.

## Riferimenti Pinterest

- [Pinterest Palette 2026](https://newsroom.pinterest.com/pdf/pinterest-palette-2026-pdf/):
  contrasto fra toni freddi, accenti vivaci e Plum Noir; quest'ultimo fornisce il
  riferimento #351E28 per la superficie del preset omonimo.
- [Dark Neon UI](https://in.pinterest.com/pin/ui_neonline--353954851938453965/):
  superfici scure, contorni luminosi e controlli leggibili.
- [Lavender Mint](https://in.pinterest.com/pin/696369161170866719/):
  abbinamenti pastello per Lavender Mist e Mint Sorbet.
- [Retro Pixel Art Computer](https://de.pinterest.com/pin/retro-pixel-art-computer--704743041686782481/):
  riferimento alla famiglia di pannelli retro con dettagli geometrici.

Sono riferimenti di palette e linguaggio visivo; effetti, pannelli e shader sono
realizzati nel codice dell'app. Nessuna immagine o risorsa dei pin viene inclusa
nell'installer. Alcuni pin consentono soltanto l'anteprima indicizzata senza login;
la palette ufficiale Pinterest rimane una fonte pubblica consultabile.
