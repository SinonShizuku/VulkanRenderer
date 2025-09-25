#pragma once
#include "DemoBase.h"
#include "../UI/ImGuiManager.h"

class DemoManager {
public:
    static DemoManager& get_singleton() {
        static DemoManager singleton = DemoManager();
        return singleton;
    }

    bool initialize(GLFWwindow* window) {
        this->window = window;
        if (!SharedResourceManager::get_singleton().initialize(window)) {
            return false;
        }
        return initialize_imgui();
    }

    bool switch_to_demo(std::unique_ptr<DemoBase> new_demo) {
        // 等待GPU完成当前操作
        if (current_demo) {
            SharedResourceManager::get_singleton().get_shared_fence().wait_and_reset();
            current_demo->cleanup_scene_resources();
        }

        if (current_demo = std::move(new_demo)) {
            return current_demo->initialize_scene_resources();
        }

        return true;
    }

    void run_main_loop() {
        if (!current_demo) {
            outstream << std::format("[ DemoManager ] ERROR\nNo demo selected!\n");
            return;
        }

        auto& shared_resources = SharedResourceManager::get_singleton();
        bool show_demo_window = true;

        while (!glfwWindowShouldClose(window)) {
            while (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                glfwWaitEvents();
            ImGuiManager::get_singleton().imgui_new_frame(show_demo_window);
            current_demo->render_ui();

            VulkanSwapchainManager::get_singleton().swap_image(
                shared_resources.get_semaphore_image_is_available()
            );

            current_demo->render_frame();

            VulkanCommand::get_singleton().submit_command_buffer_graphics(
                current_demo->get_command_buffer(),
                shared_resources.get_semaphore_image_is_available(),
                shared_resources.get_semaphore_rendering_is_over(),
                shared_resources.get_shared_fence()
            );

            VulkanCommand::get_singleton().present_image(
                shared_resources.get_semaphore_rendering_is_over()
            );

            glfwPollEvents();
            update_fps_title();

            shared_resources.get_shared_fence().wait_and_reset();
        }
    }

private:
    GLFWwindow* window = nullptr;
    std::unique_ptr<DemoBase> current_demo;

    bool initialize_imgui() {
        const auto& rpwf = VulkanPipelineManager::get_singleton().create_rpwf_screen();

        // 初始化ImGui
        ImGuiManager::get_singleton().init_basic_config();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = VulkanCore::get_singleton().get_vulkan_instance().get_instance();
        init_info.PhysicalDevice = VulkanCore::get_singleton().get_vulkan_device().get_physical_device();
        init_info.Device = VulkanCore::get_singleton().get_vulkan_device().get_device();
        init_info.QueueFamily = VulkanCore::get_singleton().get_vulkan_device().get_queue_family_index_graphics();
        init_info.Queue = VulkanCore::get_singleton().get_vulkan_device().get_queue_graphics();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = SharedResourceManager::get_singleton().get_imgui_descriptor_pool();
        init_info.RenderPass = rpwf.render_pass;
        init_info.Subpass = 0;
        init_info.MinImageCount = VulkanSwapchainManager::get_singleton().get_swapchain_create_info().minImageCount;
        init_info.ImageCount = VulkanSwapchainManager::get_singleton().get_swapchain_image_count();
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;

        return ImGui_ImplVulkan_Init(&init_info);
    }

    void update_fps_title() {
        static double time0 = glfwGetTime();
        static double time1;
        static double dt;
        static int dframe = -1;
        static std::stringstream info;
        time1 = glfwGetTime();
        dframe++;
        if ((dt = time1 - time0) >= 1) {
            info.precision(1);
            info << "Vulkan Renderer - " << (current_demo ? current_demo->get_name() : "未选择场景")
                 << "    " << std::fixed << dframe / dt << " FPS";
            glfwSetWindowTitle(window, info.str().c_str());
            info.str("");
            time0 = time1;
            dframe = 0;
        }
    }
};


