#pragma once
#include "../Start.h"
#include "../VulkanBase/VulkanCore.h"
#include "../VulkanBase/VulkanSwapchainManager.h"
#include "../VulkanBase/VulkanPipelineManager.h"
#include "../VulkanBase/components/VulkanPipepline.h"
#include "../VulkanBase/components/VulkanDescriptor.h"
#include "../VulkanBase/components/VulkanShaderModule.h"
#include "../VulkanBase/components/VulkanCommand.h"

#include "DemoCategories.h"
#include "SharedResourceManager.h"


class DemoBase {
public:
    DemoBase(DemoType type, DemoCategoryType category, const std::string& name, const std::string& description = "")
           : scene_type(type), scene_category(category), scene_name(name), scene_description(description) {}

    virtual ~DemoBase() = default;

    // Virtual functions
    virtual bool initialize_scene_resources() = 0;
    virtual void cleanup_scene_resources() = 0;
    virtual void render_frame() = 0;
    virtual void render_ui() {}

    virtual void on_window_resize(uint32_t width, uint32_t height) {}

    // Getter
    DemoType get_type() const { return scene_type; }
    DemoCategoryType get_category() const { return scene_category; }
    const std::string& get_name() const { return scene_name; }
    const std::string& get_description() const { return scene_description; }
    VkCommandBuffer get_command_buffer() const { return command_buffer; }


protected:
    DemoType scene_type;
    DemoCategoryType scene_category;
    std::string scene_name;
    std::string scene_description;

    // vulkan pipeline
    VulkanPipeline pipeline;
    VulkanPipelineLayout pipeline_layout;

    // vulkan descriptor set layout
    VulkanDescriptorSetLayout descriptor_set_layout;

    // vulkan command buffer
    VulkanCommandBuffer command_buffer;

    static const auto& get_shared_render_pass() {
        return VulkanPipelineManager::get_singleton().get_rpwf_screen().render_pass;
    }

    static const auto& get_shared_framebuffers() {
        return VulkanPipelineManager::get_singleton().get_rpwf_screen().framebuffers;
    }

    bool allocate_command_buffer() {
        return SharedResourceManager::get_singleton().get_command_pool().allocate_buffers(command_buffer);
    }

    void free_command_buffer() {
        SharedResourceManager::get_singleton().get_command_pool().free_buffers(command_buffer);
    }

};
