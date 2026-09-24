#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <stdbool.h>
#include "include/graphics.h"

int main(void) {
    te_struct_init g_core = { 0 };

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Napaka");
        return 1;
    }
    g_core.window = SDL_CreateWindow("Window", 720, 144, SDL_WINDOW_RESIZABLE);
    if (!g_core.window) {
        SDL_Log("Napaka window");
        return 1;
    }

    teVkInit(&g_core);
    teVkCreateSurface(&g_core);
    teGetPhysicalDevice(&g_core);
    teFindQueueFamilies(&g_core);
    teGetLogicalDevice(&g_core);

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
        }
    }

    return 0;
}
