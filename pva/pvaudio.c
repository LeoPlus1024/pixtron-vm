#include <SDL3/SDL.h>
#include "pvaudio.h"

#include <stdio.h>
#include <stdlib.h>


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BOX_SIZE 50


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

extern void PVA_game_loop(const PVAHandle *handle) {
    SDL_Renderer *renderer = handle->renderer;

    // 初始方块位置（窗口中心）
    SDL_FRect box = {
        (WINDOW_WIDTH - BOX_SIZE) / 2.0f,
        (WINDOW_HEIGHT - BOX_SIZE) / 2.0f,
        BOX_SIZE,
        BOX_SIZE
    };

    // 方块颜色（初始为蓝色）
    SDL_Color boxColor = {0, 100, 255, 255};

    // 主循环标志
    int running = 1;

    // 窗口尺寸
    int windowWidth = WINDOW_WIDTH;
    int windowHeight = WINDOW_HEIGHT;

    // 主循环
    while (running) {
        SDL_Event event;

        // 处理事件
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = 0;
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    // 更新方块位置（跟随鼠标）
                    box.x = event.motion.x - BOX_SIZE / 2.0f;
                    box.y = event.motion.y - BOX_SIZE / 2.0f;

                    // 根据位置改变颜色
                    boxColor.r = (Uint8)((box.x / windowWidth) * 255);
                    boxColor.g = (Uint8)((box.y / windowHeight) * 255);
                    break;

                case SDL_EVENT_KEY_DOWN:
                    // 按ESC退出
                    if (event.key.mod == SDLK_ESCAPE) {
                        running = 0;
                    }
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                    // 更新窗口尺寸
                    SDL_GetWindowSize(handle->window, &windowWidth, &windowHeight);
                    break;
            }
        }

        // 确保方块在窗口范围内
        if (box.x < 0) box.x = 0;
        if (box.y < 0) box.y = 0;
        if (box.x + box.w > windowWidth) box.x = windowWidth - box.w;
        if (box.y + box.h > windowHeight) box.y = windowHeight - box.h;

        // 清屏（深灰色背景）
        SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
        SDL_RenderClear(renderer);

        // 绘制网格背景
        SDL_SetRenderDrawColor(renderer, 50, 50, 55, 255);
        for (int y = 0; y < windowHeight; y += 20) {
            SDL_RenderLine(renderer, 0, y, windowWidth, y);
        }
        for (int x = 0; x < windowWidth; x += 20) {
            SDL_RenderLine(renderer, x, 0, x, windowHeight);
        }

        // 绘制方块
        SDL_SetRenderDrawColor(renderer, boxColor.r, boxColor.g, boxColor.b, 255);
        SDL_RenderFillRect(renderer, &box);

        // 绘制方块边框
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &box);

        // 绘制提示文本
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_FRect textBg = {10, 10, 300, 50};
        SDL_RenderFillRect(renderer, &textBg);
        SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
        SDL_RenderRect(renderer, &textBg);

        // 更新屏幕
        SDL_RenderPresent(renderer);
    }
    SDL_Quit();
}

extern void PVA_destroy(PVAHandle *handle) {
    if (handle == NULL) {
        return;
    }
    SDL_DestroyWindow(handle->window);
    SDL_DestroyRenderer(handle->renderer);
    free(handle);
}
