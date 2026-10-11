#pragma once


#include <vulkan/vulkan_core.h>
#include "graphics.h"
#include "commands.h"
#include "buffer.h"

typedef struct {
    VkFence inFlightFence;
    VkSemaphore imageAvailable, renderFinished;
} te_draw_struct;

void teDrawFuncInit(te_struct_init* core, te_draw_struct* draw);
void teDrawFuncWhile(te_struct_init* core, te_draw_struct* draw, te_command_struct* command, te_buffer_struct* buffer);

