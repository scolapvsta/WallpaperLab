#pragma once
#ifdef __cplusplus
extern "C" {
#endif
int lab_count(void);
const char *lab_title(int i);
const char *lab_preview(int i);
const char *lab_type(int i);
const char *lab_status(void);
const char *lab_error(void);
int lab_current(void);
void lab_cycle_mode(void);
void lab_cycle_screen(void);
void lab_cycle_fps(void);
void lab_toggle_mute(void);
int lab_start(int i, int preview);
int lab_poll(void);
void lab_shutdown(void);
int selector_main(int argc, char **argv);
#ifdef __cplusplus
}
#endif
