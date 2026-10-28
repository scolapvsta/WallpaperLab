# Pubblicazione di una versione

1. Implementare e verificare le modifiche in locale; preservare configurazione e wallpaper.
2. Aggiornare versione CMake e manifest, documentazione utente e test effettivi.
3. Salvare le modifiche vendor nelle patch, senza aggiornare i pin automaticamente.
4. Verificare la serie patch e gli hash. Eseguire CTest e integrazioni pertinenti.
5. Controllare `git diff --check` e i file staged: nessun asset Workshop, credenziale,
   configurazione macchina, build, screenshot o log personale.
6. Commit e push nella repository privata `scolapvsta/wallpaper-engine-customized`,
   come richiesto da Simone per le versioni successive. Mai force-push o cambio visibilità.
7. Installare il runtime verificato e confrontare le preferenze; documentare eventuali limiti.

Il codice vendor può risultare modificato localmente: la fonte riproducibile è
il pin upstream più la serie ordinata nel manifest e nello script di build.
