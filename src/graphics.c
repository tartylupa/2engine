#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <stdint.h>
#include <sys/types.h>
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>
#include "../include/graphics.h"
#include <stdlib.h>
#include <stdint.h>

static inline uint32_t clamp(uint32_t value, uint32_t min, uint32_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

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

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->physicalDevice, core->surface, &formatCount, NULL);

    VkSurfaceFormatKHR surfaceFormat[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->physicalDevice, core->surface, &formatCount, surfaceFormat);

    VkSurfaceFormatKHR choosenFormat = surfaceFormat[0];
    for (uint32_t i = 0; i < formatCount; i++) {
        if (surfaceFormat[i].format == VK_FORMAT_B8G8R8A8_SRGB &&surfaceFormat[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            choosenFormat = surfaceFormat[i];
            break;
        }
    }


    VkExtent2D extent;

    if (surfaceCapabiliteies.currentExtent.width != UINT32_MAX) {
        extent = surfaceCapabiliteies.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSizeInPixels(core->window, &width, &height);
        extent.width = (uint32_t)width;
        extent.height = (uint32_t)height;

        extent.height = clamp(extent.height, surfaceCapabiliteies.minImageExtent.height, surfaceCapabiliteies.maxImageExtent.height);
        extent.width = clamp(extent.width, surfaceCapabiliteies.minImageExtent.width, surfaceCapabiliteies.maxImageExtent.width);

    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = core->surface,
        .flags = 0,
        .minImageCount = surfaceCapabiliteies.minImageCount,
        .imageFormat = choosenFormat.format,
        .imageColorSpace = choosenFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    uint32_t queueIndices[] = { core->graphicsFamily, core->presentFamily };

    if (core->graphicsFamily != core->presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    vkCreateSwapchainKHR(core->device, &createInfo, NULL, &core->swapchain);

    return true;
}


