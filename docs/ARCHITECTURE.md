# Architettura

- `wallpaper-lab`: shell Raylib e controller C++ con API C. La modalità `--render`
  incorpora la libreria nativa del motore; non lancia renderer upstream.
- `catalog`, `project_source`, `project_properties`: catalogo Steam, risoluzione
  dei preset e proprietà persistenti per progetto.
- `workshop`: worker serializzato per Steam UGC, callback manuali, trasferimenti
  HTTPS limitati e interrompibili per le anteprime. Nessun codice remoto eseguito
  prima del download/importazione.
- `running.json`: stato con identità PID/start/exe. In modalità singola `outputs`
  associa ogni uscita a un processo indipendente; il controller sostituisce solo
  il processo del monitor selezionato. Un cambio globale sostituisce l'intero gruppo.
- `Transition.h`: postprocess OpenGL per output con snapshot del vecchio wallpaper
  e fotogramma corrente. Il controller conserva il renderer precedente fino alla
  readiness del nuovo; fallimenti di avvio non eliminano il precedente.
- `settings_ui`: menu disegnati alla fine del frame, con selezione differita e
  blocco degli eventi verso i campi coperti dall'overlay.
- `shaped_text`: cache limitata di layout Pango/Cairo; shaping Unicode e fallback
  Fontconfig. ASCII mantiene il renderer FreeType dell'interfaccia.
- `i18n`: 22 locale, fallback inglese per i messaggi avanzati delle nuove lingue.
- `patches` e `build-manifest.json`: modifiche vendor riproducibili con pin upstream.

Test e limiti effettivi sono raccolti in `tests/RESULTS.md`; build, log e dati
personali rimangono in `local/` o nelle directory XDG.

## Rendering e UI 0.14

`Transition.h` risolve il framebuffer corrente in un FBO RGBA8 e lo compone con
lo snapshot precedente. Il tempo di transizione parte dopo tre frame pronti,
non dal lancio del processo. Il guard ripristina framebuffer read/draw, viewport,
program, VAO, texture, active texture, unpack alignment e gli enable modificati.
Le coordinate dei render target sono bottom-up, quelle degli snapshot top-down.
Le texture usano clamp-to-edge; il campionamento zoom è limitato al rettangolo.
Lo shader condiviso applica saturazione, contrasto attorno a 0.5 e luminosità al
nuovo fotogramma; lo snapshot vecchio contiene già le proprie regolazioni.

`wallpaper_profiles.json[path].image` conserva brightness, contrast e saturation;
`.playback` conserva audio, reactive, mouse, parallax e particles. Le opzioni
negative globali hanno sempre precedenza su quelle locali. Il file live atomico
contiene path + image oppure path + position; il renderer lo accetta per due
secondi e poi torna ai valori persistenti. Letture limitate a una ogni 40 ms.

La UI può essere composta in un render target e presentata con alpha uniforme.
Il blending usa fattori alpha separati per non moltiplicare la trasparenza ad
ogni pannello sovrapposto. Le palette non introducono alpha nelle superfici.
Le texture Pango vengono create/rimosse solo dopo il flush del batch Raylib;
la cache LRU ospita 1.024 etichette. Le immagini Workshop restano statiche.

I gruppi risoluzione sono un OR interno, poi combinato con gli altri criteri
secondo AND/OR. Sul Workshop AND usa AddRequiredTagGroup oltre ai tag individuali;
OR usa l'elenco appiattito. Le pagine UI da 50/100/200 corrispondono a 1/2/4 query
Steam da 50. I cursori sono memorizzati per query (fino a 64 query diverse);
la navigazione sequenziale può superare il limite delle query numerate. Per
ricerche autore filtrate si conserva la scansione delle pagine utente e il
conteggio locale; il precedente limite di 1.000 pagine autore rimane.
