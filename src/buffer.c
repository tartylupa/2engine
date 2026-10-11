#include "../include/buffer.h"
#include "../include/graphics.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

void teCreateBuffer(te_buffer_struct* buffer, te_struct_init* core, VkDeviceSize bufferSize) {
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    vkCreateBuffer(core->device, &info, NULL, &buffer->buffer);

}
static uint32_t findMemoryType(te_struct_init* core, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(core->physicalDevice, &memory);

    for (uint_fast32_t i = 0; i < memory.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memory.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return 256;

}

void teAllocateMemory(te_struct_init* core, te_buffer_struct* buffer, VkDeviceSize size) {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(core->device, buffer->buffer, &memRequirements);

    VkMemoryAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = size,
        .memoryTypeIndex = findMemoryType(core, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
    };
    vkAllocateMemory(core->device, &info, NULL, &buffer->memory);
    vkBindBufferMemory(core->device, buffer->buffer, buffer->memory, 0);
}

void teCopyMemory(te_buffer_struct* buffer, te_struct_init* core, Vertex* vertices, VkDeviceSize bufferSize) {
    void *data;

    vkMapMemory(core->device, buffer->memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(core->device, buffer->memory);
}

