#include "../include/draw.h"
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


void teDrawFuncInit(te_struct_init* core, te_draw_struct* draw) {
    VkSemaphore imageAvailable, renderFinished;
    VkFence inFlightFence;

    VkSemaphoreCreateInfo semInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vkCreateSemaphore(core->device, &semInfo, NULL, &imageAvailable);
    vkCreateSemaphore(core->device, &semInfo, NULL, &renderFinished);

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    vkCreateFence(core->device, &fenceInfo, NULL, &inFlightFence);

    draw->inFlightFence = inFlightFence;
    draw->imageAvailable = imageAvailable;
    draw->renderFinished = renderFinished;
}

void teDrawFuncWhile(te_struct_init* core, te_draw_struct* draw, te_command_struct* command) {
    vkWaitForFences(core->device, 1, &draw->inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(core->device, 1, &draw->inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(core->device, core->swapchain, UINT64_MAX, draw->imageAvailable, draw->inFlightFence, &imageIndex);

    vkResetCommandBuffer(command->commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    vkBeginCommandBuffer(command->commandBuffer, &beginInfo);

    VkRenderingAttachmentInfo colorAttachement = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = core->swapchainImageView[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { .color = { { 0.0f, 0.0f, 0.0f, 1.0f } } },
    };

    VkRenderingInfo renderInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, { core->swapchainExtent.width, core->swapchainExtent.height }},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachement,
    };

    vkCmdBeginRendering(command->commandBuffer, &renderInfo);
    vkCmdBindPipeline(command->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, core->graphicsPipeline);

    vkCmdSetViewport(command->commandBuffer, 0, 1, &core->viewport);
    vkCmdSetScissor(command->commandBuffer, 0, 1, &core->scissors);

    vkCmdDraw(command->commandBuffer, 3, 1, 0, 0);
    vkCmdEndRendering(command->commandBuffer);

    vkEndCommandBuffer(command->commandBuffer);


    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &draw->imageAvailable,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &command->commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &draw->renderFinished,
    };

    vkQueueSubmit(core->graphics, 1, &submitInfo, draw->inFlightFence);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &draw->renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &core->swapchain,
        .pImageIndices = &imageIndex,
    };

    vkQueuePresentKHR(core->present, &presentInfo);
}
