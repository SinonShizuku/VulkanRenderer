#pragma once
#include "VKStart.h"
#include "VulkanCore.h"
#include "VulkanSwapchainManager.h"
#include "components/VulkanCommand.h"

class VulkanExecutionManager {
public:
    VulkanExecutionManager(){};
    ~VulkanExecutionManager(){};
    static VulkanExecutionManager& get_singleton() {
        static VulkanExecutionManager singleton = VulkanExecutionManager();
        return singleton;
    }

    result_t submit_command_buffer_graphics(VkSubmitInfo &submit_info, VkFence fence = VK_NULL_HANDLE) const {
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkResult result = vkQueueSubmit(VulkanCore::get_singleton().get_vulkan_device().get_queue_graphics(),1,&submit_info,fence);
        if (result)
            outstream << std::format("[ VulkanExecutionManager ] ERROR\nFailed to submit the command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }

    result_t submit_command_buffer_graphics(const VkCommandBuffer command_buffer,
        VkSemaphore semaphore_image_is_available = VK_NULL_HANDLE, VkSemaphore semaphore_render_is_over = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE,
        VkPipelineStageFlags wait_Dst_stage_image_is_available = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ) const {
        VkSubmitInfo submit_info = {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };
        if (semaphore_image_is_available) {
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &semaphore_image_is_available;
            submit_info.pWaitDstStageMask = &wait_Dst_stage_image_is_available;
        }
        if (semaphore_render_is_over) {
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &semaphore_render_is_over;
        }
        return submit_command_buffer_graphics(submit_info, fence);
    }

    result_t submit_command_buffer_graphics(const VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE) const {
        VkSubmitInfo submit_info = {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };
        return submit_command_buffer_graphics(submit_info, fence);
    }

    result_t submit_command_buffer_compute(VkSubmitInfo &submit_info, VkFence fence = VK_NULL_HANDLE) const {
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkResult result = vkQueueSubmit(VulkanCore::get_singleton().get_vulkan_device().get_queue_compute(),1,&submit_info,fence);
        if (result)
            outstream << std::format("[ VulkanExecutionManager ] ERROR\nFailed to submit the command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }

    result_t submit_command_buffer_compute(const VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE) const {
        VkSubmitInfo submit_info = {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };
        return submit_command_buffer_compute(submit_info, fence);
    }

    result_t present_image(VkPresentInfoKHR &present_info) {
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        switch (VkResult result = vkQueuePresentKHR(VulkanCore::get_singleton().get_vulkan_device().get_queue_presentation(),&present_info)) {
            case VK_SUCCESS:
                return VK_SUCCESS;
            case VK_SUBOPTIMAL_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                return VulkanSwapchainManager::get_singleton().recreate_swapchain();
            default:
                outstream << std::format("[ VulkanExecutionManager ] ERROR\nFailed to queue the image for presentation!\nError code: {}\n", int32_t(result));
                return result;
        }
    }

    result_t present_image(VkSemaphore semaphore_rendering_is_over = VK_NULL_HANDLE) {
        auto &swapchain = VulkanSwapchainManager::get_singleton().get_swapchain();
        auto &current_image_index = VulkanSwapchainManager::get_singleton().get_current_image_index();
        VkPresentInfoKHR present_info = {
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &current_image_index
        };
        if (semaphore_rendering_is_over) {
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &semaphore_rendering_is_over;
        }
        return present_image(present_info);
    }

};

