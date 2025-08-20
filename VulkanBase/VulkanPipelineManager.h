#pragma once
#include "../Start.h"
#include "VulkanSwapchainManager.h"
#include "components/VulkanRenderPassWithFramebuffers.h"

inline const VkExtent2D& window_size = VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageExtent;

class VulkanPipelineManager {
public:
    VulkanPipelineManager(){};
    ~VulkanPipelineManager(){};

    static VulkanPipelineManager& get_singleton() {
        static VulkanPipelineManager singleton = VulkanPipelineManager();
        return singleton;
    }

    const auto& create_rpwf_screen() {
        VkAttachmentDescription attachment_description = {
            .format = VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
        VkAttachmentReference attachment_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkSubpassDescription subpass_description = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachment_reference
        };
        VkSubpassDependency subpass_dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        };
        VkRenderPassCreateInfo render_pass_create_info = {
            .attachmentCount = 1,
            .pAttachments = &attachment_description,
            .subpassCount = 1,
            .pSubpasses = &subpass_description,
            .dependencyCount = 1,
            .pDependencies = &subpass_dependency
        };
        rpwf.render_pass.create(render_pass_create_info);

        auto create_frame_buffers = [] {
            size_t image_count = VulkanSwapchainManager::get_singleton().get_swapchain_image_count();
            rpwf.framebuffers.resize(image_count);
            VkFramebufferCreateInfo framebuffer_create_info = {
                .renderPass = rpwf.render_pass,
                .attachmentCount = 1,
                .width = window_size.width,
                .height = window_size.height,
                .layers = 1
            };
            for (auto i = 0; i < image_count; i++) {
                VkImageView attachment = VulkanSwapchainManager::get_singleton().get_swapchain_image_views()[i];
                framebuffer_create_info.pAttachments = &attachment;
                rpwf.framebuffers[i].create(framebuffer_create_info);
            }
        };

        auto destroy_framebuffers = [] {
            for (auto &framebuffer : rpwf.framebuffers) {
                framebuffer.clear();
            }
            rpwf.framebuffers.clear();
        };
        create_frame_buffers();

        ExecuteOnce(rpwf);

        VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(create_frame_buffers);
        VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(destroy_framebuffers);
        return rpwf;
    }

    void clear_rpwf_screen() {
        rpwf.render_pass.clear();
        for (auto &framebuffer : rpwf.framebuffers) {
            framebuffer.clear();
        }
        rpwf.framebuffers.clear();
    }
private:
    inline static RenderPassWithFramebuffers rpwf;

};
