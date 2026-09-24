#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <vulkan/vulkan_core.h>
#include <stdbool.h>

// struct
typedef struct te_struct_init {
    VkInstance vkInstance;
    VkSurfaceKHR surface;
    SDL_Window* window;
    VkPhysicalDevice physicalDevice;
    VkDevice device;


    // Families and queues
    uint32_t graphicsFamily;
    bool hasGraphics;

    uint32_t presentFamily;
    bool hasPresent;

    VkQueue graphics;
    VkQueue present;

    VkSwapchainKHR swapchain;

} te_struct_init;


bool teVkInit(te_struct_init* core);
bool teVkCreateSurface(te_struct_init* core);
bool teGetPhysicalDevice(te_struct_init* core);
bool teFindQueueFamilies(te_struct_init* core);
bool teGetLogicalDevice(te_struct_init *core);





#endif
