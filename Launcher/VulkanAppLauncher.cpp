#include "VulkanAppLauncher.h"
#include "../VulkanBase/VulkanCore.h"

VulkanAppLauncher* VulkanAppLauncher::singleton = nullptr;

VulkanAppLauncher& VulkanAppLauncher::getSingleton(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate) {
    if (singleton == nullptr) {
        singleton = new VulkanAppLauncher(size, fullScreen, isResizable, limitFrameRate);
    }
    return *singleton;
}

VulkanAppLauncher::VulkanAppLauncher(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate) {
    window_width = size.width;
    window_height = size.height;
    is_fullscreen = fullScreen;
    is_resizeable = isResizable;
    limit_framerate = limitFrameRate;
}

VulkanAppLauncher::~VulkanAppLauncher() {
    if (singleton != nullptr) {
        delete singleton;
        singleton = nullptr;
    }
}

void VulkanAppLauncher::run() {
    if (!init_window()) {
        outstream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize window!\n");
        return;
    }
    if (!init_vulkan()) {
        outstream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize window!\n");
        return;
    }
    while (!glfwWindowShouldClose(window)) {
        main_loop();
    }
    terminate_window();
}


bool VulkanAppLauncher::init_vulkan() {
    uint32_t extension_count = 0;
    const char** extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
    if (!extension_names) {
        outstream << std::format("[ InitializeVulkan ] ERROR\nFailed to get required extensions, Vulkan is not available on this machine!\n");
        glfwTerminate();
        return false;
    }
    // 获取拓展
    for (size_t i = 0; i < extension_count; i++) {
        VulkanCore::get_singleton().get_vulkan_instance().add_instance_extension(extension_names[i]);
    }
    VulkanCore::get_singleton().get_vulkan_device().add_device_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // 创建Vulkan实例
    VulkanCore::get_singleton().get_vulkan_instance().use_latest_api_version();
    if (VulkanCore::get_singleton().get_vulkan_instance().create_instance())
        return false;

    // 配置surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    if (VkResult result = glfwCreateWindowSurface(VulkanCore::get_singleton().get_vulkan_instance().get_instance(),window,nullptr,&surface)) {
        outstream << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
        glfwTerminate();
        return false;
    }
    VulkanCore::get_singleton().get_vulkan_instance().set_surface(surface);

    // 配置Vulkan设备
    if (VulkanCore::get_singleton().acquire_physical_devices() ||
        VulkanCore::get_singleton().determine_physical_device(0,true,false) ||
        VulkanCore::get_singleton().get_vulkan_device().create_device())
        return false;

    // 创建交换链
    if (VulkanCore::get_singleton().create_swapchain())
        return false;

    return true;
}

bool VulkanAppLauncher::init_window() {
    if (!glfwInit()) {
        outstream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
        return false;
    }
    // Vulkan格式
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, is_resizeable);

    // 配置显示器及窗口尺寸信息
    monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    window = is_fullscreen ?
        glfwCreateWindow(mode->width, mode->height,window_title,monitor,nullptr) :
        glfwCreateWindow(window_width, window_height,window_title,nullptr,nullptr);
    if (!window) {
        outstream << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
        glfwTerminate();
        return false;
    }
    return true;
}

void VulkanAppLauncher::main_loop() {
    glfwPollEvents();
    title_fps();
}

void VulkanAppLauncher::cleanup() {
    VulkanCore::get_singleton().~VulkanCore();
}

void VulkanAppLauncher::terminate_window() {
    cleanup();
    glfwTerminate();
}

void VulkanAppLauncher::title_fps() {
    static double time0 = glfwGetTime();
    static double time1;
    static double dt;
    static int dframe = -1;
    static std::stringstream info;
    time1 = glfwGetTime();
    dframe++;
    if ((dt = time1 - time0) >= 1) {
        info.precision(1);
        info << window_title << "    " << std::fixed << dframe / dt << " FPS";
        glfwSetWindowTitle(window, info.str().c_str());
        info.str("");//清空所用的stringstream
        time0 = time1;
        dframe = 0;
    }
}
