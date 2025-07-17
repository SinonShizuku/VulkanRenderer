#include "VulkanAppLauncher.h"

VulkanAppLauncher& VulkanAppLauncher::getSingleton(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate) {
    static VulkanAppLauncher singletonInstance(size, fullScreen, isResizable, limitFrameRate);
    return singletonInstance;
}

VulkanAppLauncher::VulkanAppLauncher(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate) :
    window_width(size.width),
    window_height(size.height),
    is_fullscreen(fullScreen),
    is_resizeable(isResizable),
    limit_framerate(limitFrameRate)
{
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

    main_loop();
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

    if (result_t result = glfwCreateWindowSurface(VulkanCore::get_singleton().get_vulkan_instance().get_instance(),window,nullptr,&surface)) {
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
    if (VulkanSwapchainManager::get_singleton().create_swapchain())
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
    const auto& [render_pass,framebuffers] = VulkanPipelineManager::get_singleton().create_rpwf_screen();
    create_pipeline_layout();
    create_pipeline();

    fence fence;
    semaphore semaphore_image_is_available;
    semaphore semaphore_rendering_is_over;

    command_buffer command_buffer;
    command_pool command_pool(VulkanCore::get_singleton().get_vulkan_device().get_queue_family_index_graphics(),VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    command_pool.allocate_buffers(command_buffer);

    VkClearValue clear_color = { .color = { 1.f, 1.f, 1.f, 1.f } };

    vertex vertices[] = {
        { {  .0f, -.5f }, { 1, 0, 0, 1 } },//红色
        { { -.5f,  .5f }, { 0, 1, 0, 1 } },//绿色
        { {  .5f,  .5f }, { 0, 0, 1, 1 } } //蓝色
    };
    VulkanVertexBuffer vertex_buffer(sizeof vertices);
    vertex_buffer.transfer_data(vertices);

    while (!glfwWindowShouldClose(window)) {
        while (glfwGetWindowAttrib(window,GLFW_ICONIFIED))
            glfwWaitEvents();

        VulkanSwapchainManager::get_singleton().swap_image(semaphore_image_is_available);
        auto i = VulkanSwapchainManager::get_singleton().get_current_image_index();

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            render_pass.cmd_begin(command_buffer,framebuffers[i],{{},window_size},clear_color);
            {
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer.Address(), &offset);
                vkCmdBindPipeline(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_triangle);
                vkCmdDraw(command_buffer,3,1,0,0);
            }
            render_pass.cmd_end(command_buffer);
        }
        command_buffer.end();

        VulkanExecutionManager::get_singleton().submit_command_buffer_graphics(command_buffer, semaphore_image_is_available,semaphore_rendering_is_over,fence);
        VulkanExecutionManager::get_singleton().present_image(semaphore_rendering_is_over);

        glfwPollEvents();
        title_fps();

        fence.wait_and_reset();
    }
    VulkanPipelineManager::get_singleton().clear_rpwf_screen();
}

void VulkanAppLauncher::cleanup() {
    VulkanSwapchainManager::get_singleton().destroy_singleton();
    VulkanCore::get_singleton().destroy_singleton();
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

void VulkanAppLauncher::create_pipeline_layout() {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_triangle.create(pipeline_layout_create_info);
}

void VulkanAppLauncher::create_pipeline() {
    static VulkanShaderModule vert("../Shader/VertexBuffer.vert.spv");
    static VulkanShaderModule frag("../Shader/VertexBuffer.frag.spv");
    static VkPipelineShaderStageCreateInfo shader_stage_create_infos_triangle[2] = {
        vert.stage_create_info(VK_SHADER_STAGE_VERTEX_BIT),
        frag.stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    auto create = [&] {
        GraphicsPipelineCreateInfoPack pipeline_create_info_pack;
        pipeline_create_info_pack.create_info.layout = pipeline_layout_triangle;
        pipeline_create_info_pack.create_info.renderPass = render_pass_and_frame_buffers().render_pass;
        // 子通道只有一个，pipeline_create_info_pack.createInfo.renderPass使用默认值0

        // vertex buffer
        //数据来自0号顶点缓冲区，输入频率是逐顶点输入
        pipeline_create_info_pack.vertex_input_bindings.emplace_back(0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX);
        //location为0，数据来自0号顶点缓冲区，vec2对应VK_FORMAT_R32G32_SFLOAT，用offsetof计算position在vertex中的起始位置
        pipeline_create_info_pack.vertex_input_attributes.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, position));
        //location为1，数据来自0号顶点缓冲区，vec4对应VK_FORMAT_R32G32B32A32_SFLOAT，用offsetof计算color在vertex中的起始位置
        pipeline_create_info_pack.vertex_input_attributes.emplace_back(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, color));

        pipeline_create_info_pack.input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipeline_create_info_pack.viewports.emplace_back(0.f, 0.f, float(window_width), float(window_height), 0.f, 1.f);
        pipeline_create_info_pack.scissors.emplace_back(VkOffset2D{},window_size);
        pipeline_create_info_pack.multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipeline_create_info_pack.color_blend_attachment_states.push_back({ .colorWriteMask = 0b1111 });
        pipeline_create_info_pack.update_all_arrays();
        pipeline_create_info_pack.create_info.stageCount = 2;
        pipeline_create_info_pack.create_info.pStages = shader_stage_create_infos_triangle;

        pipeline_triangle.create(pipeline_create_info_pack);
    };
    auto destroy = [this] {
        pipeline_triangle.~VulkanPipeline();
    };
    VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(create);
    VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(destroy);
    create();
}
