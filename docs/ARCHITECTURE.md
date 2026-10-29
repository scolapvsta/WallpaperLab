# Architettura

- `wallpaper-lab`: shell Raylib e controller C++ con API C. La modalità `--render`
  incorpora la libreria nativa del motore; non lancia renderer upstream.
- `catalog`, `project_source`, `project_properties`: catalogo Steam/locale, risoluzione
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
  Fontconfig. Anche ASCII usa Pango/Cairo; FreeType rimane per brand e anteprime font.
- `i18n`: 367 chiavi in 22 locale; nuove opzioni avanzate 0.16 con fallback inglese
  dove non ancora revisionate. Nomi dei temi e contenuti degli autori originali.
- `patches` e `build-manifest.json`: modifiche vendor riproducibili con pin upstream.

Test e limiti effettivi sono raccolti in `tests/RESULTS.md`; build, log e dati
personali rimangono in `local/` o nelle directory XDG.

## Rendering e UI 0.14–0.15

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

La UI è composta in RGBA quando è richiesta trasparenza. L'alpha delle superfici
dipende da `window_transparency` (0…30), mentre media e testo restano opachi.
La presentazione finale usa alpha premoltiplicato, senza un secondo fattore
globale che sbiadirebbe anche le immagini. `window_opacity` resta sincronizzata
come chiave legacy inversa; non è più il controllo esposto all'utente.
Le texture Pango vengono create/rimosse solo dopo il flush del batch Raylib;
la cache LRU ospita 1.024 etichette. Le immagini Workshop restano statiche.

I gruppi risoluzione sono un OR interno, poi combinato con gli altri criteri
secondo AND/OR. Sul Workshop AND usa AddRequiredTagGroup oltre ai tag individuali;
OR usa l'elenco appiattito. Le pagine UI da 50/100/200 corrispondono a 1/2/4 query
Steam da 50. I cursori sono memorizzati per query (fino a 64 query diverse);
la navigazione sequenziale può superare il limite delle query numerate. Per
ricerche autore filtrate si conserva la scansione delle pagine utente e il
conteggio locale; il precedente limite di 1.000 pagine autore rimane.

## Catalogo, selezione e integrazioni 0.15

`local_library.hpp` importa media tramite argv, senza shell: verifica estensione e
dimensioni con ffprobe, copia in uno staging nascosto, genera miniatura e metadata,
quindi pubblica la directory con rename. Le scansioni ignorano gli staging.
L'identità è un hash di percorso canonico, dimensione e mtime. Le immagini sono
progetti web con object-fit cover; i video mantengono il file nativo. L'asset dir
vuota gestita dall'app permette di renderizzare media locali senza asset Steam.
Scene locali collocate manualmente possono ancora richiedere gli asset upstream.
Il catalogo riconosce `lab_origin`, `lab_media_kind`, `lab_width` e `lab_height`;
in assenza del marker classifica locali i progetti sotto la radice locale.
Le radici personalizzate sostituiscono la rispettiva radice predefinita, non
migrano dati. Il source filter partecipa alla stessa combinazione AND/OR.
Picker, copie e scansioni sono job asincroni; il catalogo viene sostituito sul
thread UI dopo la conclusione dei worker delle miniature.

Il controller aggiorna il processo corrente dopo aver scoperto i monitor.
La selezione interattiva del progetto già corrente è un no-op; i cambi proprietà
che richiedono ricarica usano esplicitamente il percorso di avvio forzato.
Lo switcher conserva gli indici nell'ordine di `carousel.json`, parte dal processo
del monitor scelto e può inserire il corrente non preferito come ancora temporanea.
La prima pressione apre, le successive avanzano; il commit precoce usa la stessa
regola. La Griglia anima il contorno, senza un secondo layer ingrandito esterno.

`theme_catalog.h` definisce 16 preset indipendenti con flag light, palette, raggio
ed effetto. `theme_catalog=true` marca la migrazione dai vecchi theme_variants;
`light_theme` è ora una proprietà derivata del preset, non una variante utente.
Shader e decorazioni native producono luce, LED, riflessi, particelle e bordo RGB.
Non vengono eseguiti temi CSS esterni né importate risorse dai progetti di riferimento.

`caelestia_integration.hpp` usa solo `caelestia wallpaper --print PREVIEW`, valida
la palette e pubblica atomicamente scheme.json. Non usa wallpaper --file,
scheme set o apply_colours. Conserva la palette precedente e quella pubblicata
nello stato dell'app; al disable ripristina solo se nessuno ha cambiato la palette.
La generazione è asincrona rispetto alla UI. Prima della pubblicazione ricontrolla
flag e processo corrente sotto ControllerLock, scartando risultati tardivi dopo
disable o un altro cambio wallpaper. Gli errori vengono separati dallo stato del
renderer. La palette è unica per la shell e derivata dalla miniatura, non da ogni frame.


## Personalizzazione e generatore opzionale 0.16

Le preferenze switcher sono interi con unità esplicite (percentuali, pixel,
millisecondi): dimensione, zoom, entrata/uscita, movimento, inversione e Onda.
`popup_frame_scaled` riserva spazio per lo zoom e l'ampiezza massima senza far
crescere il pannello a ogni pressione. Il mouse usa i rettangoli disegnati nel
canvas, inclusi offset e layer ingranditi; in Griglia hover cambia selezione,
mentre nelle strisce la geometria resta stabile fino al clic. Il dot bianco è
comune al selettore e alle card. La demo dentro l'app usa PopupImages e lo stesso
shader arrotondato, senza lanciare un altro processo.

`transition_duration`, `transition_direction`, `transition_curve` e
`transition_zoom` viaggiano nella configurazione per output e negli uniform della
preview. Il motore mantiene la readiness a tre frame prima di iniziare il tempo.
`ImageTuning` estende il profilo con hue, temperature, gamma, vignette, rotation,
flip_x/flip_y. Il postprocess trasforma le UV della nuova immagine, conserva lo
snapshot già elaborato del wallpaper precedente e adatta il crop alla rotazione.
La stessa lettura profilo/live, limitata a 40 ms, aggiorna i parametri senza restart.

Le icone vengono disegnate in un atlas a 3× con margini e mipmap: cerchi ed ellissi
sono geometria piena, per evitare linee OpenGL da un pixel non scalabili. Pango/Cairo
tratta ora anche ASCII con antialiasing grayscale e hinting leggero; il batch viene
scaricato prima di creare/rimuovere texture. La composizione alpha selettiva della
0.15 resta invariata.

L'integrazione Caelestia non chiama più wallpaper --print ereditando lo schema
corrente. `caelestia_palette.py`, incluso dal componente app dell'installer,
seleziona esplicitamente uno dei quattro generatori Material You. `--prepare`
verifica i moduli già presenti o installa materialyoucolor/Pillow in un venv XDG
privato soltanto quando l'utente abilita la funzione. Registra i preset opzionali
nel data dir; non installa la shell, non usa sudo e non modifica il pacchetto Caelestia.

Il controller richiede un frame fresco al proprio renderer (PID e path verificati),
lo scrive temporaneamente in PPM e lo rimuove dopo la generazione. Se non arriva,
usa la miniatura del progetto. I ruoli surface/background ricevono una tinta
contenuta dal seed, così i widget mostrano il colore anche con superfici Material
normalmente poco sature. L'output resta dynamic/default per compatibilità con la
shell. Il callback di pubblicazione verifica anche variante/modo correnti; una
modifica in coda causa una nuova generazione. Il ripristino condizionale 0.15
rimane valido. Il browser statico di Caelestia non viene popolato né riavviato.
