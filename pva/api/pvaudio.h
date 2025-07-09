#ifndef ENGINE_H
#define ENGINE_H

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDLHandle;

extern SDLHandle *pva_engine_init(const char *title, int w, int h, int flags);


extern void pva_engine_destroy(SDLHandle *handle);

#endif
