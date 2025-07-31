#pragma once
#include "../VulkanBase/VKStart.h"
#include "../VulkanBase/VulkanCore.h"
#include "../VulkanBase/VulkanExecutionManager.h"
#include "../VulkanBase/VulkanPipelineManager.h"
#include "../VulkanBase/VulkanSwapchainManager.h"
#include "../VulkanBase/components/VulkanCommand.h"
#include "../VulkanBase/components/VulkanSync.h"
#include "../VulkanBase/components/VulkanPipepline.h"
#include "../VulkanBase/components/VulkanShaderModule.h"
#include "../VulkanBase/components/VulkanBuffers.h"
#include "../Scene/Vertex.h"
#include "../VulkanBase/components/VulkanDescriptor.h"

class VulkanAppLauncher {
public:
    static VulkanAppLauncher& getSingleton(VkExtent2D size = {800, 600}, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = false);
    ~VulkanAppLauncher() = default;
    void run();

private:
    VulkanAppLauncher(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate);
    VulkanAppLauncher(const VulkanAppLauncher&) = delete;
    VulkanAppLauncher& operator=(const VulkanAppLauncher&) = delete;

    GLFWwindow* window;
    GLFWmonitor* monitor;
    // 窗口信息
    uint32_t window_width, window_height;
    const char* window_title = "Vulkan Renderer";
    bool is_resizeable;
    bool is_fullscreen;
    bool limit_framerate;

    // 管线
    VulkanPipelineLayout pipeline_layout_triangle;
    VulkanPipeline pipeline_triangle;

    // 描述符集
    VulkanDescriptorSetLayout descriptor_set_layout_triangle;

    bool init_vulkan();
    bool init_window();
    // void init_assets(vertex vertices[], uint16_t indices[], glm::vec2 pushConstants[]);

    void main_loop();
    void cleanup();
    void terminate_window();
    void title_fps();

    const auto& render_pass_and_frame_buffers() {
        static const auto& rpwf = VulkanPipelineManager::get_singleton().create_rpwf_screen();
        return rpwf;
    }

    void create_pipeline_layout();
    void create_pipeline_layout_with_push_constant();
    void create_pipeline_layout_with_uniform_buffer();
    void create_pipeline();
};
