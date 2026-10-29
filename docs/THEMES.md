# Themes, Color Palettes e modalità — 0.20

I temi sono ora UI complete. In Aspetto le tre scelte sono indipendenti:

- **Themes**: Originale (backup) oppure Strawberry core.
- **Color Palettes**: le 29 palette precedenti, con ID invariati, più Strawberry
  Milk, Raspberry Ribbon e Vanilla Shortcake (32 in totale).
- **Mode**: Dark oppure Light, senza cambiare il tema o la palette selezionati.

Il cambio di tema ricorda separatamente palette e modalità di ciascuna UI.
Alla prima selezione Strawberry core usa Strawberry Milk in Light mode.
Le impostazioni precedenti migrano a Originale mantenendo aspetto e colori.
Una copia locale dei sorgenti della 0.19 e delle preferenze precedenti al rework
è conservata in `local/backups/themes-0.19/`; la cronologia Git conserva la versione.

## Strawberry core

Fondo vichy rosa con fragole e piccoli fiocchi, coniglietti con guance rosa e
nastrini, testata decorata, pizzo sui pannelli e tasti a biscotto con glassa,
bordi smerlati e puntinatura. Anche i toggle usano una base a biscotto. I tre
abbinamenti dedicati variano dal latte e fragola al lampone e alla vaniglia.
La versione scura usa superfici cacao/rosa e accenti più chiari.

Le illustrazioni sono vettoriali originali generate dal codice raylib: i tre
riferimenti forniti in chat ispirano motivi, colori e accessori, senza includere
le fotografie o l'illustrazione con personaggi nell'app. Nessun asset remoto,
font o servizio online è necessario. Le decorazioni sono statiche e rispettano
Riduci animazioni; i quattro layout del carousel mantengono la loro struttura.
Anteprime Workshop, foto personali e scene live conservano il proprio contenuto.

I colori degli altri preset in modalità opposta sono derivati dal loro accento:
non sono 32 coppie di palette disegnate manualmente. Strawberry è la prima UI
rielaborata; gli stili precedenti sono ancora disponibili in Originale. Il popup
transitorio del carousel mantiene il layout esistente e riceve i colori, senza
la cornice decorata della finestra principale. Il wallpaper desktop non cambia.

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
