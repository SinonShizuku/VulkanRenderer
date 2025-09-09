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
{}

void VulkanAppLauncher::run() {
    VulkanCommand::get_singleton();

    if (!init_window()) {
        outstream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize window!\n");
        return;
    }
    if (!init_vulkan()) {
        outstream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize window!\n");
        return;
    }

    // VulkanExecutionManager::get_singleton().boot_screen("../Assets/img.png", VK_FORMAT_R8G8B8A8_UNORM);
    // std::this_thread::sleep_for(std::chrono::seconds(1LL));

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
    auto api_version = VulkanCore::get_singleton().get_vulkan_instance().get_api_version();
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
        VulkanCore::get_singleton().get_vulkan_device().create_device(api_version))
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
    create_pipeline_layout_with_texture();
    create_pipeline();

    fence fence;
    semaphore semaphore_image_is_available;
    semaphore semaphore_rendering_is_over;

    VulkanCommandBuffer command_buffer;
    VulkanCommandPool command_pool(VulkanCore::get_singleton().get_vulkan_device().get_queue_family_index_graphics(),VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    command_pool.allocate_buffers(command_buffer);

    // load texture image
    VulkanTexture2D texture_image("../Assets/img.png", VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, true);
    VkSamplerCreateInfo sampler_create_info = VulkanTexture2D::get_sampler_create_info();
    VulkanSampler sampler(sampler_create_info);

    VkClearValue clear_color = { .color = { 1.f, 1.f, 1.f, 1.f } };

    // vertex vertices[] = {
    //     { { .0f, -.5f }, { 1, 0, 0, 1 } },
    //     { { -.5f, .5f }, { 0, 1, 0, 1 } },
    //     { { .5f, .5f }, { 0, 0, 1, 1 } }
    // };
    // uint16_t indices[] = {
    //     0, 1, 2,
    //     1, 2, 3
    // };
    // push_constant_data_3 push_constant = {
    //     {  .0f, .0f },
    //     { -.5f, .0f },
    //     {  .5f, .0f },
    // };
    // glm::vec2 uniform_positions[] = {
    //     {  .0f, .0f }, {},
    //     { -.5f, .0f }, {},
    //     {  .5f, .0f }, {}
    // };
    // VulkanVertexBuffer vertex_buffer(sizeof vertices);
    // VulkanIndexBuffer index_buffer(sizeof indices);
    // vertex_buffer.transfer_data(vertices);
    // index_buffer.transfer_data(indices);
    // VulkanUniformBuffer uniform_buffer(sizeof uniform_positions);
    // uniform_buffer.transfer_data(uniform_positions);

    texture_vertex vertices[] = {
        { { -1.f, -1.f }, { 0, 0 } },
        { {  1.f, -1.f }, { 1, 0 } },
        { { -.5f,  .5f }, { 0, 1 } },
        { {  .5f,  .5f }, { 1, 1 } }
    };
    VulkanVertexBuffer vertex_buffer(sizeof vertices);
    vertex_buffer.transfer_data(vertices);

    // 创建descriptor pool
    VkDescriptorPoolSize descriptor_pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
    };
    VulkanDescriptorPool descriptor_pool(1, descriptor_pool_sizes);

    // 分配descriptor set
    VulkanDescriptorSet descriptor_set_texture;
    descriptor_pool.allocate_sets(descriptor_set_texture,descriptor_set_layout_texture);

    VkDescriptorImageInfo image_info = {
        .sampler = sampler,
        .imageView = texture_image.get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    descriptor_set_texture.write(image_info,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    // // 将uniform buffer写入描述符
    // VkDescriptorBufferInfo buffer_info = {
    //     .buffer = uniform_buffer,
    //     .offset = 0,
    //     .range = sizeof uniform_positions,
    // };
    // descriptor_set_triangle_position.write(buffer_info,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    while (!glfwWindowShouldClose(window)) {
        while (glfwGetWindowAttrib(window,GLFW_ICONIFIED))
            glfwWaitEvents();

        VulkanSwapchainManager::get_singleton().swap_image(semaphore_image_is_available);
        auto i = VulkanSwapchainManager::get_singleton().get_current_image_index();

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            render_pass.cmd_begin(command_buffer,framebuffers[i],{{},window_size},clear_color);
            {
                // bind utils
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer.Address(), &offset);
                // vkCmdBindIndexBuffer(command_buffer,index_buffer,0,VK_INDEX_TYPE_UINT16);
                vkCmdBindPipeline(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_texture);
                // vkCmdPushConstants(command_buffer,pipeline_layout_triangle,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof push_constant, &push_constant);
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline_layout_texture, 0, 1, descriptor_set_texture.Address(), 0, nullptr);
                // vkCmdBindDescriptorSets(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_layout_triangle, 0, 1, descriptor_set_triangle_position.Address(), 0, nullptr);

                // draw
                // vkCmdDraw(command_buffer,3,1,1,0);
                // vkCmdDrawIndexed(command_buffer,6,1,0,0,0);
                vkCmdDraw(command_buffer, 4, 1, 0, 0);
            }
            render_pass.cmd_end(command_buffer);
        }
        command_buffer.end();

        VulkanCommand::get_singleton().submit_command_buffer_graphics(command_buffer, semaphore_image_is_available,semaphore_rendering_is_over,fence);
        VulkanCommand::get_singleton().present_image(semaphore_rendering_is_over);

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

void VulkanAppLauncher::create_pipeline_layout_with_push_constant() {
    VkPushConstantRange push_constant_range = {
        VK_SHADER_STAGE_VERTEX_BIT,
        0,//offset
        24//范围大小，3个vec2是24
    };
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range,
    };
    pipeline_layout_triangle.create(pipeline_layout_create_info);
}

void VulkanAppLauncher::create_pipeline_layout_with_uniform_buffer() {
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .bindingCount = 1,
        .pBindings = &descriptor_set_layout_binding
    };
    descriptor_set_layout_triangle.create(descriptor_set_layout_create_info);
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .setLayoutCount = 1,
        .pSetLayouts = descriptor_set_layout_triangle.Address()
    };
    pipeline_layout_triangle.create(pipeline_layout_create_info);
}

void VulkanAppLauncher::create_pipeline_layout_with_texture() {
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .bindingCount = 1,
        .pBindings = &descriptor_set_layout_binding
    };
    descriptor_set_layout_texture.create(descriptor_set_layout_create_info);
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .setLayoutCount = 1,
        .pSetLayouts = descriptor_set_layout_texture.Address()
    };
    pipeline_layout_texture.create(pipeline_layout_create_info);
}

void VulkanAppLauncher::create_pipeline() {
    static VulkanShaderModule vert("../Shader/Texture.vert.spv");
    static VulkanShaderModule frag("../Shader/Texture.frag.spv");
    // static VkPipelineShaderStageCreateInfo shader_stage_create_infos_triangle[2] = {
    //     vert.stage_create_info(VK_SHADER_STAGE_VERTEX_BIT),
    //     frag.stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT)
    // };
    static VkPipelineShaderStageCreateInfo shader_stage_create_infos_texture[2] = {
        vert.stage_create_info(VK_SHADER_STAGE_VERTEX_BIT),
        frag.stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    auto create = [&] {
        GraphicsPipelineCreateInfoPack pipeline_create_info_pack;
        pipeline_create_info_pack.create_info.layout = pipeline_layout_texture;
        pipeline_create_info_pack.create_info.renderPass = render_pass_and_frame_buffers().render_pass;
        // 子通道只有一个，pipeline_create_info_pack.createInfo.renderPass使用默认值0

        // vertex buffer
        //数据来自0号顶点缓冲区，输入频率是逐顶点输入
        pipeline_create_info_pack.vertex_input_bindings.emplace_back(0, sizeof(texture_vertex), VK_VERTEX_INPUT_RATE_VERTEX);
        // //location为0，数据来自0号顶点缓冲区，vec2对应VK_FORMAT_R32G32_SFLOAT，用offsetof计算position在vertex中的起始位置
        // pipeline_create_info_pack.vertex_input_attributes.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, position));
        // //location为1，数据来自0号顶点缓冲区，vec4对应VK_FORMAT_R32G32B32A32_SFLOAT，用offsetof计算color在vertex中的起始位置
        // pipeline_create_info_pack.vertex_input_attributes.emplace_back(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, color));
        pipeline_create_info_pack.vertex_input_attributes.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(texture_vertex, position));
        pipeline_create_info_pack.vertex_input_attributes.emplace_back(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(texture_vertex, texCoord));

        // pipeline_create_info_pack.input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipeline_create_info_pack.input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        pipeline_create_info_pack.viewports.emplace_back(0.f, 0.f, float(window_width), float(window_height), 0.f, 1.f);
        pipeline_create_info_pack.scissors.emplace_back(VkOffset2D{},window_size);
        pipeline_create_info_pack.multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipeline_create_info_pack.color_blend_attachment_states.push_back({ .colorWriteMask = 0b1111 });
        pipeline_create_info_pack.update_all_arrays();
        pipeline_create_info_pack.create_info.stageCount = 2;
        pipeline_create_info_pack.create_info.pStages = shader_stage_create_infos_texture;

        // pipeline_triangle.create(pipeline_create_info_pack);
        pipeline_texture.create(pipeline_create_info_pack);
    };
    auto destroy = [this] {
        pipeline_triangle.~VulkanPipeline();
    };
    VulkanSwapchainManager::get_singleton().add_callback_create_swapchain(create);
    VulkanSwapchainManager::get_singleton().add_callback_destroy_swapchain(destroy);
    create();
}
