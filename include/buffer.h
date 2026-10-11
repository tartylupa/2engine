#pragma once

#include "graphics.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;

} te_buffer_struct;


typedef struct {
    float x, y, z;

} Vertex;



void teCopyMemory(te_buffer_struct* buffer, te_struct_init* core, Vertex* vertices, VkDeviceSize bufferSize);
void teAllocateMemory(te_struct_init* core, te_buffer_struct* buffer, VkDeviceSize size);
void teCreateBuffer(te_buffer_struct* buffer, te_struct_init* core, VkDeviceSize bufferSize);
