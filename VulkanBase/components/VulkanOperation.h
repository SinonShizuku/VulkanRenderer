#pragma once
#include "VulkanSync.h"
#include "../../Start.h"
#include "../VulkanCore.h"
#include "../VulkanSwapchainManager.h"

struct image_operation {
    struct image_memory_barrier_parameter_pack {
        const bool is_needed = false;
        const VkPipelineStageFlags stage = 0;  // srcStages或dstStages
        const VkAccessFlags access = 0;
        const VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

        constexpr image_memory_barrier_parameter_pack() = default;
        constexpr image_memory_barrier_parameter_pack(const VkPipelineStageFlags stage, const VkAccessFlags access, const VkImageLayout layout)
            : is_needed(true), stage(stage), access(access), layout(layout) {}
    };

    static void cmd_copy_buffer_to_image(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, const VkBufferImageCopy& region,
        image_memory_barrier_parameter_pack imb_from, image_memory_barrier_parameter_pack imb_to) {
        VkImageMemoryBarrier image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            imb_from.access,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            imb_from.layout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            image,
            {
                region.imageSubresource.aspectMask,
                region.imageSubresource.mipLevel,
                1,
                region.imageSubresource.baseArrayLayer,
                region.imageSubresource.layerCount
            }
        };
        if (imb_from.is_needed) {
            vkCmdPipelineBarrier(command_buffer, imb_from.stage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }
        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        if (imb_to.is_needed) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_memory_barrier.dstAccessMask = imb_to.access;
            image_memory_barrier.newLayout = imb_to.layout;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, imb_to.stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }
    }

    static void cmd_blit_image(VkCommandBuffer command_buffer, VkImage image_src, VkImage image_dst, const VkImageBlit& region,
        image_memory_barrier_parameter_pack imb_dst_from, image_memory_barrier_parameter_pack imb_dst_to, VkFilter filter = VK_FILTER_LINEAR) {
        VkImageMemoryBarrier image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            imb_dst_from.access,
            VK_ACCESS_TRANSFER_READ_BIT,
            imb_dst_from.layout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            image_dst,
            {
                region.dstSubresource.aspectMask,
                region.dstSubresource.mipLevel,
                1,
                region.dstSubresource.baseArrayLayer,
                region.dstSubresource.layerCount
            }
        };
        if (imb_dst_from.is_needed) {
            vkCmdPipelineBarrier(command_buffer, imb_dst_from.stage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }
        vkCmdBlitImage(command_buffer,
            image_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region, filter);
        if (imb_dst_to.is_needed) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_memory_barrier.dstAccessMask = imb_dst_to.access;
            image_memory_barrier.newLayout = imb_dst_to.layout;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, imb_dst_to.stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }
    }

    static void cmd_generate_mipmap2d(VkCommandBuffer command_buffer, VkImage image, VkExtent2D image_extent, uint32_t mip_level_count, uint32_t layer_count,
        image_memory_barrier_parameter_pack imb_to, VkFilter min_filter = VK_FILTER_LINEAR) {
        auto get_mipmap_extent = [](VkExtent2D image_extent, uint32_t mip_level) {
            VkOffset3D extent = { int32_t(image_extent.width >> mip_level), int32_t(image_extent.height >> mip_level), 1 };
            extent.x += !extent.x;
            extent.y += !extent.y;
            return extent;
        };
        for (uint32_t i=1; i<mip_level_count; i++) {
            VkImageBlit region = {
                { VK_IMAGE_ASPECT_COLOR_BIT, i-1, 0, layer_count },
                {{},get_mipmap_extent(image_extent, i-1)},
                { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, layer_count },
                {{},get_mipmap_extent(image_extent, i)}
            };
            cmd_blit_image(command_buffer, image, image, region,
                { VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
                min_filter);
        }
        if (imb_to.is_needed) {
            VkImageMemoryBarrier image_memory_barrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                nullptr,
                0,
                imb_to.access,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                imb_to.layout,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                image,
                { VK_IMAGE_ASPECT_COLOR_BIT, 0, mip_level_count, 0, layer_count }
            };
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, imb_to.stage, 0,
                0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }
    }
};


