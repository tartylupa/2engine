#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../include/graphics.h"
#include "../include/shaders.h"


typedef struct {
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
} te_command_struct;


int teCreateCommandBuffer(te_command_struct* command, te_struct_init* core);
int teCreateCommandPool(te_command_struct* command, te_struct_init* core);


