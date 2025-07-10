#include <SDL3/SDL.h>
#include "pvaudio.h"

#include <stdlib.h>

struct _PVAHandle {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

extern PVAHandle *PVA_init(const char *title, const int w, const int h, const int flags) {
    const bool init = SDL_Init(flags);
    if (!init) {
        SDL_Log("SDL_Init() Error: %s", SDL_GetError());
        return NULL;
    }
    SDL_Window *window = SDL_CreateWindow(title, w, h, flags);
    if (!window) {
        SDL_Log("SDL_Init() Error: %s", SDL_GetError());
        return NULL;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Log("SDL_CreateRenderer() Error: ", SDL_GetError());
        return NULL;
    }
    PVAHandle *handle = malloc(sizeof(PVAHandle));
    if (handle == NULL) {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        return NULL;
    }
    handle->window = window;
    handle->renderer = renderer;
    return handle;
}

extern bool PVA_window_show(const PVAHandle *handle) {
    return SDL_ShowWindow(handle->window);
}

extern void PVA_destroy(PVAHandle *handle) {
    if (handle == NULL) {
        return;
    }
    SDL_DestroyWindow(handle->window);
    SDL_DestroyRenderer(handle->renderer);
    free(handle);
}
