#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_vulkan.h>
#include <stdint.h>
#include <sys/types.h>
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>
#include "../include/graphics.h"
#include <stdlib.h>



bool teVkInit(te_struct_init* core) {

    uint32_t extensionCount;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);


    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "mango engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
        .pNext = NULL,
        .pApplicationName = "mango"
    };
    VkInstanceCreateInfo instanceCreateInfo = {
        .pApplicationInfo = &appInfo,
        .pNext = NULL,
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledLayerCount = 0,
        .flags = 0,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions
    };
    vkCreateInstance(&instanceCreateInfo, NULL, &core->vkInstance);

    return true;
}

bool teVkCreateSurface(te_struct_init* core) {
    if (!SDL_Vulkan_CreateSurface(core->window, core->vkInstance, NULL, &core->surface)) {
        return false;
    }

    return true;
}

bool teGetPhysicalDevice(te_struct_init* core) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(core->vkInstance, &count, NULL);

    if (count == 0) {
        SDL_Log("No GPU found");
        return false;
    }
    VkPhysicalDevice device[count];
    vkEnumeratePhysicalDevices(core->vkInstance, &count, device);

    uint32_t currentScore = 0;
    int deviceCountChoosen = -1;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t score = 0;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device[i], &properties);

        switch (properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: score += 10000; break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score += 1000; break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: score += 100; break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU: score += 10; break;
            default: score += 1; break;
        }

        // + score for max 2D image and clip distance
        score += properties.limits.maxClipDistances * 1000 + properties.limits.maxImageDimension2D;

        if (score > currentScore) {
            currentScore = score;
            deviceCountChoosen = i;
        }
    }

    if (deviceCountChoosen == -1) {
        SDL_Log("No GPU found! Or good one");
        return false;
    }
    core->physicalDevice = device[deviceCountChoosen];

    return true;
}

bool teFindQueueFamilies(te_struct_init* core) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(core->physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(core->physicalDevice, &queueFamilyCount, queueFamilies);

    core->hasGraphics = false;

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            core->hasGraphics = true;
            core->graphicsFamily = i;
        }

        VkBool32 presentMode = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(core->physicalDevice, i, core->surface, &presentMode);
        if (presentMode) {
            core->hasPresent = true;
            core->presentFamily = i;
        }

        if (core->hasGraphics && core->hasPresent) {
            break;
        }

    }

    return true;
}

bool teGetLogicalDevice(te_struct_init *core) {
    uint32_t uniqueQueueFamilies[2];
    uint32_t queueFamilyCount = 0;

    uniqueQueueFamilies[queueFamilyCount++] = core->graphicsFamily;
    if (core->graphicsFamily != core->presentFamily) {
        uniqueQueueFamilies[queueFamilyCount++] = core->presentFamily;
    }

    VkDeviceQueueCreateInfo queueCreateInfo[queueFamilyCount];
    float queuePriority = 1.0f;

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        queueCreateInfo[i] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueQueueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
    }
    VkPhysicalDeviceVulkan13Features fetures13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = VK_TRUE,
        .synchronization2 = VK_TRUE,
    };
    VkPhysicalDeviceFeatures2 deviceFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &fetures13,
    };

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };




    VkDeviceCreateInfo createinfo = {
        .pNext = &deviceFeatures2,
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfo,
        .queueCreateInfoCount = queueFamilyCount,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions,
    };
    if (vkCreateDevice(core->physicalDevice, &createinfo, NULL, &core->device) != VK_SUCCESS) {
        return false;
    }

    vkGetDeviceQueue(core->device, core->graphicsFamily, 0, &core->graphics);
    vkGetDeviceQueue(core->device, core->presentFamily, 0, &core->present);


    return true;
}

bool teCreateSwapchain(te_struct_init* core) {
    VkSurfaceCapabilitiesKHR surfaceCapabiliteies;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->physicalDevice, core->surface, &surfaceCapabiliteies);

    

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = core->surface,
        .flags = 0,
        .minImageCount = surfaceCapabiliteies.minImageCount,
    };

    vkCreateSwapchainKHR(core->device, &createInfo, NULL, &core->swapchain);
}


