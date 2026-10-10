#include "include/draw.h"
#include "include/standard.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include "include/graphics.h"
#include "include/shaders.h"

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
    teCreateSwapchain(&g_core);
    teCreateImageViews(&g_core);

    te_shader_struct vertShader;
    size_t vertSize;
    char* vertexShader = loadToRampChar("shaders/main.vert.spv", &vertSize);
    teCreateShaderModule(vertexShader, &vertShader, vertSize, &g_core);

    te_shader_struct fragShader;
    size_t fragSize;
    char* fragmentShader = loadToRampChar("shaders/main.frag.spv", &fragSize);
    teCreateShaderModule(fragmentShader, &fragShader, fragSize, &g_core);

    if (!teCreateDynamicStatePipeline(&g_core, &vertShader, &fragShader)) {
        printf("Dynamic State Pipeline func error\n");
    }

    te_command_struct cmdStruct;
    teCreateCommandPool(&cmdStruct, &g_core);
    teCreateCommandBuffer(&cmdStruct, &g_core);

    te_draw_struct draw;
    teDrawFuncInit(&g_core, &draw);

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
        }
        teDrawFuncWhile(&g_core, &draw, &cmdStruct);


    }

    return 0;
}
