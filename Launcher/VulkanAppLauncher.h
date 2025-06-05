#pragma once
#include "../VulkanBase/VKStart.h"

class VulkanAppLauncher {
public:
    static VulkanAppLauncher& getSingleton(VkExtent2D size = {800, 600}, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = false);
    ~VulkanAppLauncher();
    void run();

private:
    VulkanAppLauncher(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate);
    VulkanAppLauncher(const VulkanAppLauncher&) = delete;
    VulkanAppLauncher& operator=(const VulkanAppLauncher&) = delete;

    static VulkanAppLauncher* singleton;

    GLFWwindow* window;
    GLFWmonitor* monitor;
    // 窗口信息
    uint32_t window_width, window_height;
    const char* window_title = "Vulkan Renderer";
    bool is_resizeable;
    bool is_fullscreen;
    bool limit_framerate;

    bool init_vulkan();
    bool init_window();
    void main_loop();
    void cleanup();
    void terminate_window();
    void title_fps();
};
