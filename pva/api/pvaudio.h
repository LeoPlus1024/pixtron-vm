#ifndef ENGINE_H
#define ENGINE_H

typedef struct _PVAHandle PVAHandle;

extern PVAHandle *PVA_init(const char *title, int w, int h, int flags);

extern bool PVA_window_show(const PVAHandle *handle);

extern void PVA_destroy(PVAHandle *handle);

#endif
