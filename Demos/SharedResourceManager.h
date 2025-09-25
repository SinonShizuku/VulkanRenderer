#pragma once
#include "../Start.h"
#include "../VulkanBase/VulkanCore.h"
#include "../VulkanBase/components/VulkanSync.h"
#include "../VulkanBase/components/VulkanCommand.h"
#include "../VulkanBase/components/VulkanDescriptor.h"
#include "../UI/ImGuiManager.h"

class SharedResourceManager {
public:
    static SharedResourceManager& get_singleton() {
        static SharedResourceManager singleton = SharedResourceManager();
        return singleton;
    }

    bool initialize(GLFWwindow* window) {
        this->window = window;
        shared_fence = std::make_unique<fence>();
        semaphore_image_is_available = std::make_unique<semaphore>();
        semaphore_rendering_is_over = std::make_unique<semaphore>();

        // 创建共享命令池
        command_pool = std::make_unique<VulkanCommandPool>(
            VulkanCore::get_singleton().get_vulkan_device().get_queue_family_index_graphics(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );

        return initialize_imgui_resources();
    }

    // Getter
    fence& get_shared_fence() { return *shared_fence; }
    semaphore& get_semaphore_image_is_available() { return *semaphore_image_is_available; }
    semaphore& get_semaphore_rendering_is_over() { return *semaphore_rendering_is_over; }
    VulkanCommandPool& get_command_pool() { return *command_pool; }
    VulkanDescriptorPool& get_imgui_descriptor_pool() { return *imgui_descriptor_pool; }
    GLFWwindow* get_window() { return window; }

private:
    GLFWwindow* window = nullptr;

    // 共享同步对象
    std::unique_ptr<fence> shared_fence;
    std::unique_ptr<semaphore> semaphore_image_is_available;
    std::unique_ptr<semaphore> semaphore_rendering_is_over;

    // 共享命令池
    std::unique_ptr<VulkanCommandPool> command_pool;

    // ImGui资源
    std::unique_ptr<VulkanDescriptorPool> imgui_descriptor_pool;

    bool initialize_imgui_resources() {
        // ImGui描述符池配置
        VkDescriptorPoolSize imgui_pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        imgui_descriptor_pool = std::make_unique<VulkanDescriptorPool>(
            1000 * IM_ARRAYSIZE(imgui_pool_sizes),
            imgui_pool_sizes
        );
        return true;
    }
};
