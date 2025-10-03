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
    DemoBase(DemoType type, DemoCategoryType category,  const std::string& description = "")
           : scene_type(type), scene_category(category),  scene_description(description) {}

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
    const std::string& get_description() const { return scene_description; }
    VkCommandBuffer get_command_buffer() const { return command_buffer; }

    // Setter
    void set_window(GLFWwindow *window) { this->window = window; }

    // Swapchain callback management
    void add_swapchain_create_callback(std::function<void()> create_callback) {
        swapchain_create_callbacks.push_back(std::move(create_callback));
    }
    
    void add_swapchain_destroy_callback(std::function<void()> destroy_callback) {
        swapchain_destroy_callbacks.push_back(std::move(destroy_callback));
    }
    
    void register_swapchain_callbacks() {
        for (auto& callback : swapchain_create_callbacks) {
            VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(callback);
        }
        for (auto& callback : swapchain_destroy_callbacks) {
            VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(callback);
        }
    }
    
    void clear_swapchain_callbacks() {
        // Clear all swapchain callbacks from the manager since we can't identify specific ones
        VulkanSwapchainManager::get_singleton().clear_all_callbacks();
    }


protected:
    GLFWwindow *window;
    DemoType scene_type;
    DemoCategoryType scene_category;
    std::string scene_description;

    // vulkan pipeline
    VulkanPipeline pipeline;
    VulkanPipelineLayout pipeline_layout;

    // vulkan descriptor set layout
    VulkanDescriptorSetLayout descriptor_set_layout;

    // vulkan command buffer
    VulkanCommandBuffer command_buffer;

    // swapchain callbacks for this demo
    std::vector<std::function<void()>> swapchain_create_callbacks;
    std::vector<std::function<void()>> swapchain_destroy_callbacks;
    
    // callback ID management
    std::vector<uint64_t> registered_callback_ids;
    static inline uint64_t next_callback_id = 0;

    static const auto& get_shared_render_pass() {
        return VulkanPipelineManager::get_singleton().get_rpwf_screen().render_pass;
    }

    static const auto& get_shared_framebuffers() {
        return VulkanPipelineManager::get_singleton().get_rpwf_screen().framebuffers;
    }

    static const auto& get_shared_render_pass_imageless_framebuffer() {
        return VulkanPipelineManager::get_singleton().get_rpwf_screen_imageless_framebuffer().render_pass;
    }

    static const auto& get_shared_imageless_framebuffer() {
        return VulkanPipelineManager::get_singleton().get_rpwf_screen_imageless_framebuffer().framebuffer;
    }

    static const auto& get_shared_render_pass_offscreen() {
        return VulkanPipelineManager::get_singleton().get_rpwf_offscreen().render_pass;
    }

    static const auto& get_shared_framebuffer_offscreen() {
        return VulkanPipelineManager::get_singleton().get_rpwf_offscreen().framebuffer;
    }

    bool allocate_command_buffer() {
        return SharedResourceManager::get_singleton().get_command_pool().allocate_buffers(command_buffer);
    }

    void free_command_buffer() {
        SharedResourceManager::get_singleton().get_command_pool().free_buffers(command_buffer);
    }

};
