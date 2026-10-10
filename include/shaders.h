#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "graphics.h"

typedef struct {
    VkShaderModule shaderModule;
} te_shader_struct;



int teCreateDynamicStatePipeline(te_struct_init* core, te_shader_struct* vertShader, te_shader_struct* fragShader);
int teCreateShaderPipeline(te_shader_struct* shader, te_struct_init* core, VkShaderStageFlagBits stage);
int teCreateShaderModule(char* source, te_shader_struct* shader, size_t size, te_struct_init* core);
