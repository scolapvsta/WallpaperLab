# Versione Windows — valutazione 0.17

Questa è una stima progettuale, non una versione Windows già disponibile né un
impegno di calendario. Il codice attuale è verificato su Arch Linux/Hyprland;
non basta ricompilarlo per Windows o avviarlo con Wine.

## Stima indicativa

Con una persona a tempo pieno, una macchina Windows reale e test continui:

- **4–6 settimane** per un prototipo con interfaccia, libreria, importazione di
  immagini/video e applicazione desktop di base.
- **3–5 mesi complessivi** per una beta ampia con più monitor, scene/web,
  audio reattivo, media correnti, transizioni, Workshop e installer, avvicinandosi
  alle funzioni Linux attuali. Le compatibilità dei progetti restano da verificare.

I tempi includono sviluppo e prove, non una pubblicazione immediata. Driver,
rendering CEF/mpv e comportamento di Explorer possono allungarli; non è stata
ancora eseguita una compilazione Windows del progetto.

## Parti riutilizzabili e lavoro necessario

Catalogo, profili, traduzioni, parte della UI Raylib, parsing delle scene e
shader possono essere riutilizzati. Il progetto applicativo è invece collegato
a processi POSIX, `/proc`, segnali, lock, percorsi XDG, librerie `.so`, Hyprland e
alla selezione delle uscite Linux. Il motore ha alcune condizioni CMake Windows
per CEF, ma manca un backend desktop Windows completo in questa integrazione.

Servono adattatori per hosting dello sfondo e ripristino dopo il riavvio di
Explorer, processi/lifecycle, scorciatoie globali, selezione dei file, monitor e
DPI differenti, caricamento delle dipendenze e packaging. L'audio di sistema
richiede un percorso Windows come
[WASAPI loopback](https://learn.microsoft.com/en-us/windows/win32/coreaudio/loopback-recording);
la gestione dei monitor deve rispettare la
[consapevolezza DPI del processo](https://learn.microsoft.com/en-us/windows/win32/hidpi/setting-the-default-dpi-awareness-for-a-process)
e i vincoli di
[hosting di finestre con DPI differenti](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows).
Anche metadati dei player e integrazione Steam richiedono implementazione e prove
su Windows. Caelestia rimarrebbe un extra Linux.

La priorità iniziale ragionevole è separare le API del sistema operativo, fare
compilare la UI con un renderer immagini/video e aggiungere poi le scene. Nessun
port Windows è stato avviato nella 0.17: la richiesta era una valutazione dei tempi.
