# Themes, font e switcher — 0.23

| Theme | Modalità | Palette | Font suggerito iniziale | Background inclusi |
| --- | --- | ---: | --- | ---: |
| Default | Scura | 3 | Sans di sistema | 4 |
| Strawberry core | Chiara | 3 | Sans di sistema | 4 |
| 2077 | Scura | 11 | Oxanium | 4 |
| Pixel Theme | Scura | 4 | Silkscreen | 4 |

Themes e Color Palettes sono dropdown. Le palette dipendono dal tema; la modalità
è stabilita dal tema. La nuova installazione parte da Default/Graphite. Gli ID
Strawberry e 2077 esistenti restano invariati; palette e background sono ricordati
separatamente per ciascun tema. I vecchi preset anteriori al rework rimangono nel
backup locale e nella cronologia, fuori dal catalogo attivo.

## Default

UI sobria, superfici morbide e bordi discreti. Palette **Graphite**, **Blue Slate**,
**Forest Ink**. Fondi **Quiet Gradient**, **Blueprint**, **Contour**, **Night Dots**:
gradiente, piccoli segni a griglia, curve di livello o punti. Font suggeriti:
Sans, Adwaita Sans e Noto Sans, con fallback di sistema dove necessario.

## Strawberry core

**Strawberry Milk**, **Raspberry Ribbon**, **Vanilla Shortcake**. I quattro sfondi
ora cambiano composizione: **Strawberry Gingham** a quadretti, **Ribbon Atelier**
con nastri diagonali e grandi fiocchi, **Strawberry Garden** crema con fragole,
fiori e coniglietto, **Tea Party Lace** con fasce di pizzo e piccoli centrini.

I biscotti sono capsule lisce con estremità interamente arrotondate; tolte le
piccole smerlature che rendevano irregolare il bordo. Hover con luce e bordo rosa,
pressione più scura e selezione evidenziata. Icone ridotte per rimanere dentro i
biscotti e niente anello della stella all'esterno del tasto.

## 2077

**Violet Afterglow**, **Pink Voltage**, **Electric Blue**, **Night Drive**,
**Hazard Yellow**, **Crimson Protocol**, più cinque nuove palette: **Toxic Lime**,
**Amber Circuit**, **Ice Chrome**, **Ultraviolet**, **Copper Sunset**.

Skyline al neon, pioggia, LED e bit; fondi **Neon District**, **Afterlife Alley**,
**Binary Rain**, **Edge Signal**. La testata sinistra usa una composizione compatta
WALLPAPER LAB / CONTROL DECK; il marchio destro 2077 è più grande, con separazione
cromatica e fasce glitch per circa 0,32 s ogni 1,8 s. Riduci animazioni li ferma.
L'accento dei pulsanti è un piccolo segno laterale, solo per i controlli di testo:
nessun segno sui pulsanti con icona, né la vecchia striscia su ogni pannello.

## Pixel Theme

UI a pixel, pannelli squadrati con ombra a gradini, sprite originali (avventuriero,
slime, robot e fantasma), nuvole, torce, stelle e piccoli glitch. Non sono sprite
estratti da videogiochi. Palette **Arcade Berry**, **Dungeon Moss**, **Coin Rush**,
**CRT Ocean**. Background **Overworld**, **Dungeon**, **Arcade**, **Star Quest**.
Animazioni rispettano Riduci animazioni. Silkscreen è incluso con licenza SIL OFL,
provenienza e SHA-256 nel manifest; font alternativi suggeriti Oxanium e DejaVu Sans Mono.

## Suggested Theme Fonts e General Fonts

Subito sotto Themes:

1. **Suggested Theme Fonts** sceglie il font pertinente al tema, ricordato per tema.
2. **General Fonts** contiene tutte le famiglie della lista dell'app, più la spunta
   affiancata. Menu e spunta occupano insieme la stessa colonna degli altri campi.

La scelta General Fonts è salvata anche quando la spunta è spenta, ma non viene
applicata. Solo con la spunta attiva sostituisce il font suggerito in tutta la UI,
comprese le testate e il popup dello switcher. Spegnendola ritorna immediatamente
il font suggerito. I font personali già salvati rimangono selezionabili nel menu
General Fonts; il nuovo override è inizialmente spento. Nessuna installazione globale.

2077 suggerisce Oxanium, Rajdhani e Orbitron; Strawberry e Default Sans, Adwaita Sans,
Noto Sans. Font scaricati dal repository ufficiale Google Fonts al commit fissato in
`assets/fonts/manifest.json`, con licenze OFL nel runtime. Sono font per l'app, non
font estratti dal gioco o dall'anime. Font non disponibili e alfabeti non coperti
usano il fallback Fontconfig/Pango.

- [Silkscreen, fonte ufficiale](https://github.com/google/fonts/tree/main/ofl/silkscreen)
- [Oxanium](https://github.com/google/fonts/tree/main/ofl/oxanium)
- [Orbitron](https://github.com/google/fonts/tree/main/ofl/orbitron)
- [Rajdhani](https://github.com/google/fonts/tree/main/ofl/rajdhani)
- [Edgerunners, riferimento d'atmosfera](https://www.cyberpunk.net/en/edgerunners)

## Switcher coerenti con la UI

**Griglia, Fiume, Verticale e Onda** condividono il disegno fra anteprima e popup
reale: palette delle cornici, pannello, tagli degli angoli e decorazioni seguono
il tema. Strawberry aggiunge fiocchi/pizzo; 2077 angoli a taglio e neon; Pixel
cornici squadrate e sprite; Default una cornice sobria. La scelta General Fonts
attiva vale anche nel popup. Immagini e preview dei wallpaper conservano i propri
colori: si modifica la cornice, non si ricolora il contenuto.

Le forme e gli effetti non cambiano ordine, rotazione, hitbox, dimensionamento o
logica di commit. Onda conserva il fondo trasparente attorno alle carte. Il fondo
dello switcher è una versione compatta del tema, non una copia integrale dello
sfondo personalizzato importato per la finestra principale.

## App background dal PC

Dropdown con quattro fondi inclusi e Carica dal PC, preview vicina e pulsanti
compatti Carica dal PC / Ripristina tema. Il file viene copiato come PNG statico
entro 2048 × 2048 nei dati XDG `wallpaper-lab/app-backgrounds/`; l'originale rimane
intatto e può essere spostato. Ripristina tema torna al primo preset del tema.
Un'immagine non valida conserva la selezione precedente. Il fondo importato è
comune ai temi e non viene importato nel catalogo wallpaper desktop. Miniature
costruite su uno scenario 1280 × 720 per includere anche i decori in basso.
GIF importate al primo frame; servono zenity/kdialog e FFmpeg come per i media locali.

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
