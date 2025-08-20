#pragma once
#include "../Start.h"
#include "VKFormat.h"
#include "VulkanSwapchainManager.h"
#include "components/VulkanCommand.h"
#include "components/VulkanSync.h"
#include "../Scene/Texture.h"
#include "components/VulkanOperation.h"
#include "components/VulkanBuffers.h"



class VulkanExecutionManager {
public:
    VulkanExecutionManager(){};
    ~VulkanExecutionManager(){};

    static VulkanExecutionManager& get_singleton() {
        static VulkanExecutionManager singleton = VulkanExecutionManager();
        return singleton;
    }

    void boot_screen(const char* image_path, VkFormat image_format) {
        VkExtent2D image_extent;
        std::unique_ptr<uint8_t[]> p_image_data = Texture::load_file(image_path, image_extent, VulkanCore::get_singleton().get_vulkan_device().get_format_info(image_format));
        if (!p_image_data) {
            outstream << std::format("[ VulkanExecutionManager ] ERROR\nFailed to load the image file!\nFile path: {}\n", image_path);
            return;
        }
        VulkanStagingBuffer::buffer_data_main_thread(p_image_data.get(), image_extent.width * image_extent.height * VulkanCore::get_singleton().get_vulkan_device().get_format_info(image_format).sizePerPixel);

        // 同步
        semaphore semaphore_image_is_available;
        fence fence;

        VulkanCommandBuffer command_buffer;
        VulkanCommand::get_singleton().get_command_pool_graphics().allocate_buffers(command_buffer);

        VulkanSwapchainManager::get_singleton().swap_image(semaphore_image_is_available);

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            VkExtent2D swapchain_image_size  = VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageExtent;

            VulkanImageMemory image_memory;
            // BLock Image Transfer (块图像传输)
            bool blit =
                image_extent.width != swapchain_image_size.width ||                       //宽
                image_extent.height != swapchain_image_size.height ||                     //高
                image_format != VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageFormat; //图像格式

            if (blit) {
                VkImage image = VulkanStagingBuffer::aliased_image2d_main_thread(image_format, image_extent);
                if (image) {
                    VkImageMemoryBarrier image_memory_barrier = {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        nullptr,
                        0,
                        VK_ACCESS_TRANSFER_READ_BIT,
                        VK_IMAGE_LAYOUT_PREINITIALIZED,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_QUEUE_FAMILY_IGNORED,
                        VK_QUEUE_FAMILY_IGNORED,
                        image,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
                    };
                    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr, 0, nullptr, 1, &image_memory_barrier);
                }
                else {
                    VkImageCreateInfo create_info = {
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = image_format,
                        .extent = { image_extent.width, image_extent.height, 1 },
                        .mipLevels = 1,
                        .arrayLayers = 1,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                    };
                    image_memory.create(create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    VkBufferImageCopy region_copy = {
                        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                        .imageExtent = { image_extent.width, image_extent.height, 1 }
                    };
                    image_operation::cmd_copy_buffer_to_image(command_buffer,
                        VulkanStagingBuffer::get_buffer_main_thread(),
                        image_memory.Image(),
                        region_copy,
                        { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                        { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL });
                    image = image_memory.Image();
                }
                VkImageBlit region_blit = {
                    .srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                    .srcOffsets = { {0, 0, 0}, {int32_t(image_extent.width), int32_t(image_extent.height), 1} },
                    .dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                    .dstOffsets = { {0, 0, 0}, {int32_t(swapchain_image_size.width), int32_t(swapchain_image_size.height), 1}}
                 };
                image_operation::cmd_blit_image(command_buffer,
                    image,
                    VulkanSwapchainManager::get_singleton().get_swapchain_image(VulkanSwapchainManager::get_singleton().get_current_image_index()),
                    region_blit,
                    { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED  },
                    { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
                    VK_FILTER_NEAREST
                );
            }
            else {
                VkBufferImageCopy region_copy = {
                    .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                    .imageExtent = { image_extent.width, image_extent.height, 1}
                };
                image_operation::cmd_copy_buffer_to_image(command_buffer,
                    VulkanStagingBuffer::get_buffer_main_thread(),
                    VulkanSwapchainManager::get_singleton().get_swapchain_image(VulkanSwapchainManager::get_singleton().get_current_image_index()),
                    region_copy,
                    { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                    { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR}
                    );
            }
        }
        command_buffer.end();

        VkPipelineStageFlags wait_dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo submit_info = {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = semaphore_image_is_available.Address(),
            .pWaitDstStageMask = &wait_dst_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = command_buffer.Address()
        };
        VulkanCommand::get_singleton().submit_command_buffer_graphics(submit_info, fence);

        fence.wait_and_reset();
        VulkanCommand::get_singleton().present_image();

        VulkanCommand::get_singleton().get_command_pool_graphics().free_buffers(command_buffer);
    }

};

