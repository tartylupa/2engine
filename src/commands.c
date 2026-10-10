#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../include/commands.h"
#include "../include/graphics.h"


int teCreateCommandPool(te_command_struct* command, te_struct_init* core) {
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = core->graphicsFamily,
    };

    if (vkCreateCommandPool(core->device, &info, NULL, &command->commandPool) != VK_SUCCESS) {
        return 0;
    }

    return 1;
}

int teCreateCommandBuffer(te_command_struct* command, te_struct_init* core) {
    VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command->commandPool,
        .commandBufferCount = 1,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };
    vkAllocateCommandBuffers(core->device, &info, &command->commandBuffer);

    return 1;
}
