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

    const auto& get_rpwf_imgui() {
        return rpwf_imgui;
    }

    const auto& get_rpwf_screen() {
        return rpwfs;
    }

    const auto& get_rpwf_screen_imageless_framebuffer() {
        return rpwf;
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
        rpwfs.render_pass.create(render_pass_create_info);

        auto create_frame_buffers = [] {
            size_t image_count = VulkanSwapchainManager::get_singleton().get_swapchain_image_count();
            rpwfs.framebuffers.resize(image_count);
            VkFramebufferCreateInfo framebuffer_create_info = {
                .renderPass = rpwfs.render_pass,
                .attachmentCount = 1,
                .width = window_size.width,
                .height = window_size.height,
                .layers = 1
            };
            for (auto i = 0; i < image_count; i++) {
                VkImageView attachment = VulkanSwapchainManager::get_singleton().get_swapchain_image_views()[i];
                framebuffer_create_info.pAttachments = &attachment;
                rpwfs.framebuffers[i].create(framebuffer_create_info);
            }
        };

        auto destroy_framebuffers = [] {
            for (auto &framebuffer : rpwfs.framebuffers) {
                framebuffer.clear();
            }
            rpwfs.framebuffers.clear();
        };
        create_frame_buffers();

        ExecuteOnce(rpwfs);

        VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(create_frame_buffers);
        VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(destroy_framebuffers);
        return rpwfs;
    }

    void clear_rpwf_screen() {
        rpwfs.render_pass.clear();
        for (auto &framebuffer : rpwfs.framebuffers) {
            framebuffer.clear();
        }
        rpwfs.framebuffers.clear();
    }

    const auto& create_rpwf_imgui() {
        VkAttachmentDescription attachment_description = {
            .format = VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
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
        rpwf_imgui.render_pass.create(render_pass_create_info);

        auto create_frame_buffers = [] {
            size_t image_count = VulkanSwapchainManager::get_singleton().get_swapchain_image_count();
            rpwf_imgui.framebuffers.resize(image_count);
            VkFramebufferCreateInfo framebuffer_create_info = {
                .renderPass = rpwf_imgui.render_pass,
                .attachmentCount = 1,
                .width = window_size.width,
                .height = window_size.height,
                .layers = 1
            };
            for (auto i = 0; i < image_count; i++) {
                VkImageView attachment = VulkanSwapchainManager::get_singleton().get_swapchain_image_views()[i];
                framebuffer_create_info.pAttachments = &attachment;
                rpwf_imgui.framebuffers[i].create(framebuffer_create_info);
            }
        };

        auto destroy_framebuffers = [] {
            for (auto &framebuffer : rpwf_imgui.framebuffers) {
                framebuffer.clear();
            }
            rpwf_imgui.framebuffers.clear();
        };
        create_frame_buffers();

        ExecuteOnce(rpwf_imgui);

        VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(create_frame_buffers);
        VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(destroy_framebuffers);
        return rpwf_imgui;
    }

    void clear_rpwf_imgui() {
        rpwf_imgui.render_pass.clear();
        for (auto &framebuffer : rpwf_imgui.framebuffers) {
            framebuffer.clear();
        }
        rpwf_imgui.framebuffers.clear();
    }

    const auto& create_rpwf_screen_imageless_framebuffer() {
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

        // 待填充
        auto create_framebuffer = [] {
            VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
                .usage = VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageUsage,
                .width = window_size.width,
                .height = window_size.height,
                .layerCount = 1,
                .viewFormatCount = 1,
                .pViewFormats = &VulkanSwapchainManager::get_singleton().get_swapchain_create_info().imageFormat
            };
            if (VulkanSwapchainManager::get_singleton().get_swapchain_create_info().flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)
                framebuffer_attachment_image_info.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
            if (VulkanSwapchainManager::get_singleton().get_swapchain_create_info().flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)
                framebuffer_attachment_image_info.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
            if (VulkanSwapchainManager::get_singleton().get_swapchain_create_info().flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)
                framebuffer_attachment_image_info.flags |=  VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;;
            VkFramebufferAttachmentsCreateInfo framebuffer_attachments_create_info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
                .attachmentImageInfoCount = 1,
                .pAttachmentImageInfos = &framebuffer_attachment_image_info
            };
            VkFramebufferCreateInfo framebuffer_create_info = {
                .pNext = &framebuffer_attachments_create_info,
                .flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
                .renderPass = rpwf.render_pass,
                .attachmentCount = 1,
                .width = window_size.width,
                .height = window_size.height,
                .layers = 1
            };
            rpwf.framebuffer.create(framebuffer_create_info);
        };
        auto destroy_framebuffer = [] {
            rpwf.framebuffer.~VulkanFramebuffer();
        };
        create_framebuffer();

        ExecuteOnce(rpwf);
        VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(create_framebuffer);
        VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(destroy_framebuffer);

        return rpwf;
    }

    void clear_rpwf_screen_imagelss_framebuffer() {
        rpwf.render_pass.clear();
        rpwf.framebuffer.clear();
    }

    void clear_all_rpwf() {
        clear_rpwf_screen();
        clear_rpwf_imgui();
        clear_rpwf_screen_imagelss_framebuffer();
    }
private:
    inline static RenderPassWithFramebuffers rpwfs;
    inline static RenderPassWithFramebuffers rpwf_imgui;
    inline static RenderPassWithFramebuffer rpwf;

};
