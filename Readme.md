# Vulkan Renderer 项目概述

这是一个基于 Vulkan API 的渲染器项目，旨在提供一个基础的图形渲染框架。项目结构清晰，将 Vulkan 的各个组件封装成独立的类，便于管理和扩展。

## 文件结构

```
.
├── CMakeLists.txt
├── main.cpp
├── Start.h
├── stb_image_implementation.cpp
├── Assets/
│   └── img.png
├── cmake-build-debug/
├── Launcher/
│   ├── VulkanAppLauncher.cpp
│   └── VulkanAppLauncher.h
├── Scene/
│   ├── Texture.h
│   └── Vertex.h
├── Shader/
│   ├── FirstTriangle.frag.shader
│   ├── FirstTriangle.frag.spv
│   ├── FirstTriangle.vert.shader
│   ├── FirstTriangle.vert.spv
│   ├── PushConstant.vert.shader
│   ├── PushConstant.vert.spv
│   ├── UniformBuffer.vert.shader
│   ├── UniformBuffer.vert.spv
│   ├── VertexBuffer.frag.shader
│   └── VertexBuffer.frag.spv
│   └── VertexBuffer.vert.shader
│   └── VertexBuffer.vert.spv
└── VulkanBase/
    ├── VKFormat.h
    ├── VulkanContext.h
    ├── VulkanCore.h
    ├── VulkanExecutionManager.h
    ├── VulkanPipelineManager.h
    ├── VulkanResourceManager.h
    ├── VulkanSwapchainManager.h
    └── components/
        ├── VulkanBuffers.h
        ├── VulkanCommand.h
        ├── VulkanDescriptor.h
        ├── VulkanDevice.h
        ├── VulkanInstance.h
        ├── VulkanOperation.h
        ├── VulkanPipepline.h
        ├── VulkanRenderPassWithFramebuffers.h
        ├── VulkanShaderModule.h
        └── VulkanSync.h
```

## 核心类与功能

### `Start.h`

*   **概述**: 包含项目全局宏定义、常用头文件（如 GLM, GLFW, Vulkan）、以及一些通用工具类和函数（如 `result_t` 用于 Vulkan 结果封装，`array_ref` 用于数组引用，`between_closed` 用于数值范围判断）。
*   **关键类/结构体**:
    *   `result_t`: 用于封装 `VkResult`，根据宏定义（`VK_RESULT_THROW`, `VK_RESULT_NODISCARD`）决定是否抛出异常或发出警告。
    *   `array_ref<T>`: 一个简单的数组引用封装类，提供指针和计数。
    *   `default_window_size`: 全局常量，定义默认窗口大小。
    *   `outstream`: 全局输出流，默认为 `std::cout`。
*   **关键宏**:
    *   `DestroyHandleBy(device, Func)`: 用于安全销毁 Vulkan 句柄。
    *   `MoveHandle`: 用于实现移动语义。
    *   `DefineMoveAssignmentOperator(type)`: 定义移动赋值运算符。
    *   `DefineHandleTypeOperator`: 定义到句柄类型的转换运算符。
    *   `DefineAddressFunction`: 定义获取句柄地址的函数。
    *   `ExecuteOnce(...)`: 确保代码块只执行一次。

### `main.cpp`

*   **概述**: 项目的入口点，负责创建 `VulkanAppLauncher` 单例并启动渲染循环。
*   **关键函数**:
    *   `main()`: 程序主函数，调用 `VulkanAppLauncher::getSingleton().run()` 启动应用。

### `Launcher/VulkanAppLauncher.h` / `Launcher/VulkanAppLauncher.cpp`

*   **概述**: 应用程序的启动器类，负责窗口管理、Vulkan 初始化、主渲染循环和资源清理。
*   **关键类**:
    *   `VulkanAppLauncher`:
        *   **关键函数**:
            *   `getSingleton()`: 获取 `VulkanAppLauncher` 的单例实例。
            *   `run()`: 启动应用程序，包括窗口和 Vulkan 初始化，然后进入主循环。
            *   `init_window()`: 初始化 GLFW 窗口。
            *   `init_vulkan()`: 初始化 Vulkan 实例、物理设备、逻辑设备、表面和交换链。
            *   `main_loop()`: 主渲染循环，处理事件、渲染帧。
            *   `cleanup()`: 清理 Vulkan 资源。
            *   `terminate_window()`: 终止窗口并清理 GLFW。
            *   `title_fps()`: 更新窗口标题显示 FPS。
            *   `create_pipeline_layout()`: 创建 Vulkan 管线布局。
            *   `create_pipeline_layout_with_push_constant()`: 创建带 Push Constant 的管线布局。
            *   `create_pipeline_layout_with_uniform_buffer()`: 创建带 Uniform Buffer 的管线布局。
            *   `create_pipeline()`: 创建图形管线。

### `Scene/Vertex.h`

*   **概述**: 定义了顶点结构体和 Push Constant 数据结构。
*   **关键结构体**:
    *   `vertex`: 定义顶点数据，包含 `position` (glm::vec2) 和 `color` (glm::vec4)。
    *   `push_constant_data_3`: 定义用于 Push Constant 的数据结构，包含三个 `glm::vec2` 偏移量。

### `Scene/Texture.h`

*   **概述**: 提供了加载图像文件的静态函数，支持从文件路径或内存二进制数据加载图像，并处理不同的图像格式。
*   **关键类**:
    *   `Texture`:
        *   **关键静态函数**:
            *   `load_file_internal()`: 内部函数，使用 `stb_image.h` 加载图像数据。
            *   `load_file(const char* file_path, ...)`: 从文件路径加载图像。
            *   `load_file(const uint8_t* file_binaries, ...)`: 从内存二进制数据加载图像。

### `VulkanBase/VKFormat.h`

*   **概述**: 定义了 `VulkanFormatInfo` 结构体，用于描述 Vulkan 格式的详细信息，并提供了 `format_infos_v1_0` 数组，映射 Vulkan 格式到其具体属性。
*   **关键结构体**:
    *   `VulkanFormatInfo`: 包含 `componentCount` (通道数), `sizePerComponent` (每个通道大小), `sizePerPixel` (每个像素大小), `rawDataType` (底层数据类型)。
*   **关键常量**:
    *   `format_infos_v1_0[]`: 存储 Vulkan 1.0 版本中所有格式的详细信息。

### `VulkanBase/VulkanCore.h`

*   **概述**: 作为 Vulkan 核心组件的单例管理器，负责 Vulkan 实例和逻辑设备的生命周期管理，以及物理设备的获取和队列族索引的确定。
*   **关键类**:
    *   `VulkanCore`:
        *   **关键函数**:
            *   `get_singleton()`: 获取 `VulkanCore` 的单例实例。
            *   `destroy_singleton()`: 销毁所有 Vulkan 核心资源（实例、设备、表面、调试信使）。
            *   `wait_idle()`: 等待设备空闲。
            *   `get_vulkan_instance()`: 获取 `VulkanInstance` 引用。
            *   `get_vulkan_device()`: 获取 `VulkanDevice` 引用。
            *   `acquire_physical_devices()`: 获取所有可用的物理设备。
            *   `get_queue_family_indices()`: 获取指定物理设备的队列族索引（图形、呈现、计算）。
            *   `determine_physical_device()`: 确定要使用的物理设备和其队列族索引。
*   **成员变量**:
    *   `vulkan_instance`: `VulkanInstance` 类的实例。
    *   `vulkan_device`: `VulkanDevice` 类的实例。

### `VulkanBase/VulkanExecutionManager.h`

*   **概述**: 管理 Vulkan 的执行流程，特别是启动屏幕（boot screen）的图像传输操作。
*   **关键类**:
    *   `VulkanExecutionManager`:
        *   **关键函数**:
            *   `get_singleton()`: 获取 `VulkanExecutionManager` 的单例实例。
            *   `boot_screen(const char* image_path, VkFormat image_format)`: 加载图像并将其传输到交换链图像上作为启动屏幕。处理图像的缩放和格式转换。

### `VulkanBase/VulkanPipelineManager.h`

*   **概述**: 管理 Vulkan 渲染通道和帧缓冲区的创建与清理。
*   **关键类**:
    *   `VulkanPipelineManager`:
        *   **关键函数**:
            *   `get_singleton()`: 获取 `VulkanPipelineManager` 的单例实例。
            *   `create_rpwf_screen()`: 创建用于屏幕渲染的渲染通道和帧缓冲区。
            *   `clear_rpwf_screen()`: 清理屏幕渲染相关的渲染通道和帧缓冲区。
*   **成员变量**:
    *   `rpwf`: 静态成员，`RenderPassWithFramebuffers` 结构体，包含渲染通道和帧缓冲区列表。

### `VulkanBase/VulkanResourceManager.h`

*   **概述**: (此文件内容被注释掉，可能已废弃或正在开发中)

### `VulkanBase/VulkanSwapchainManager.h`

*   **概述**: 管理 Vulkan 交换链的创建、重建、图像获取和呈现。
*   **关键类**:
    *   `VulkanSwapchainManager`:
        *   **关键函数**:
            *   `get_singleton()`: 获取 `VulkanSwapchainManager` 的单例实例。
            *   `destroy_singleton()`: 销毁交换链及其相关资源。
            *   `wait_idle()`: 等待设备空闲。
            *   `set_surface_formats()`: 设置表面格式。
            *   `get_surface_formats()`: 获取物理设备支持的表面格式。
            *   `create_swapchain()`: 创建 Vulkan 交换链。
            *   `recreate_swapchain()`: 重建交换链（例如，当窗口大小改变时）。
            *   `recreate_device()`: 重建逻辑设备。
            *   `swap_image()`: 获取下一个可用的交换链图像。
            *   `add_callback_create_swapchain()`: 添加交换链创建回调函数。
            *   `add_callback_destroy_swapchain()`: 添加交换链销毁回调函数。
*   **成员变量**:
    *   `swapchain`: Vulkan 交换链句柄。
    *   `swapchain_images`: 交换链图像列表。
    *   `swapchain_image_views`: 交换链图像视图列表。
    *   `swapchain_create_info`: 交换链创建信息。

### `VulkanBase/components/VulkanBuffers.h`

*   **概述**: 封装了 Vulkan 内存、缓冲区和图像的创建、绑定和数据传输操作。
*   **关键类**:
    *   `VulkanDeviceMemory`: 封装 `VkDeviceMemory`，提供内存分配、映射、解映射和数据传输功能。
        *   **关键函数**: `allocate()`, `map_memory()`, `unmap_memory()`, `buffer_data()`, `retrieve_data()`。
    *   `VulkanBuffer`: 封装 `VkBuffer`，提供缓冲区创建和内存绑定功能。
        *   **关键函数**: `create()`, `memory_allocate_info()`, `bind_memory()`。
    *   `VulkanBufferMemory`: 继承自 `VulkanBuffer` 和 `VulkanDeviceMemory`，组合了缓冲区和内存管理。
        *   **关键函数**: `create_buffer()`, `allocate_memory()`, `bind_memory()`, `create()`。
    *   `VulkanBufferView`: 封装 `VkBufferView`，提供缓冲区视图的创建。
        *   **关键函数**: `create()`。
    *   `VulkanImage`: 封装 `VkImage`，提供图像创建和内存绑定功能。
        *   **关键函数**: `create()`, `memory_alloc_info()`, `bind_memory()`。
    *   `VulkanImageMemory`: 继承自 `VulkanImage` 和 `VulkanDeviceMemory`，组合了图像和内存管理。
        *   **关键函数**: `create_image()`, `allocate_memory()`, `bind_memory()`, `create()`。
    *   `VulkanImageView`: 封装 `VkImageView`，提供图像视图的创建。
        *   **关键函数**: `create()`。
    *   `VulkanStagingBuffer`: 管理一个用于数据传输的暂存缓冲区，支持主线程和静态单例访问。
        *   **关键函数**: `expand()`, `release()`, `map_memory()`, `unmap_memory()`, `write_buffer_data()`, `retrieve_data()`, `aliased_image2d()`。
        *   **关键静态函数**: `get_buffer_main_thread()`, `buffer_data_main_thread()` 等。
    *   `VulkanDeviceLocalBuffer`: 继承自 `VulkanBufferMemory`，用于创建设备本地缓冲区，并提供数据传输方法。
        *   **关键函数**: `transfer_data()`, `create()`, `recreate()`。
    *   `VulkanVertexBuffer`: 继承自 `VulkanDeviceLocalBuffer`，专门用于顶点缓冲区。
    *   `VulkanIndexBuffer`: 继承自 `VulkanDeviceLocalBuffer`，专门用于索引缓冲区。
    *   `VulkanUniformBuffer`: 继承自 `VulkanDeviceLocalBuffer`，专门用于 Uniform 缓冲区。
        *   **关键静态函数**: `calculate_aligned_size()`。

### `VulkanBase/components/VulkanCommand.h`

*   **概述**: 封装了 Vulkan 命令缓冲区和命令池的创建、分配、记录和提交操作。
*   **关键类**:
    *   `VulkanCommandBuffer`: 封装 `VkCommandBuffer`，提供命令缓冲区的开始和结束记录功能。
        *   **关键函数**: `begin()`, `end()`。
    *   `VulkanCommandPool`: 封装 `VkCommandPool`，提供命令池的创建、命令缓冲区的分配和释放。
        *   **关键函数**: `create()`, `allocate_buffers()`, `free_buffers()`。
    *   `VulkanCommand`: 作为命令管理器的单例，管理图形、计算和呈现队列的命令池和命令缓冲区，并提供提交和呈现图像的功能。
        *   **关键函数**:
            *   `get_singleton()`: 获取 `VulkanCommand` 的单例实例。
            *   `submit_command_buffer_graphics()`: 提交图形命令缓冲区。
            *   `submit_command_buffer_compute()`: 提交计算命令缓冲区。
            *   `submit_command_buffer_presentation()`: 提交呈现命令缓冲区。
            *   `present_image()`: 呈现图像到屏幕。
            *   `cmd_transfer_image_ownership()`: 传输图像所有权。
            *   `execute_command_buffer_graphics()`: 执行图形命令缓冲区并等待完成。
            *   `acquire_image_ownership_presentation()`: 获取图像呈现所有权。

### `VulkanBase/components/VulkanDescriptor.h`

*   **概述**: 封装了 Vulkan 描述符集布局、描述符集和描述符池的创建、分配和更新操作。
*   **关键类**:
    *   `VulkanDescriptorSetLayout`: 封装 `VkDescriptorSetLayout`，提供描述符集布局的创建。
        *   **关键函数**: `create()`。
    *   `VulkanDescriptorSet`: 封装 `VkDescriptorSet`，提供向描述符集写入（更新）描述符的功能。
        *   **关键函数**: `write()` (重载用于图像、缓冲区、纹素缓冲区信息)。
        *   **关键静态函数**: `update()` (用于批量更新描述符集)。
    *   `VulkanDescriptorPool`: 封装 `VkDescriptorPool`，提供描述符池的创建、描述符集的分配和释放。
        *   **关键函数**: `create()`, `allocate_sets()`, `free_sets()`。

### `VulkanBase/components/VulkanDevice.h`

*   **概述**: 封装了 Vulkan 逻辑设备和物理设备的管理，包括设备创建、队列获取、设备扩展检查以及物理设备属性和格式属性的查询。
*   **关键类**:
    *   `VulkanDevice`:
        *   **关键函数**:
            *   `get_device()`: 获取逻辑设备句柄。
            *   `get_physical_device()`: 获取物理设备句柄。
            *   `get_physical_device_properties()`: 获取物理设备属性。
            *   `get_physical_device_memory_properties()`: 获取物理设备内存属性。
            *   `get_queue_family_index_graphics()` / `_presentation()` / `_compute()`: 获取对应队列族索引。
            *   `get_queue_graphics()` / `_presentation()` / `_compute()`: 获取对应队列句柄。
            *   `get_format_properties()`: 获取指定格式的属性。
            *   `get_format_info()`: 获取指定格式的详细信息（基于 `VKFormat.h`）。
            *   `add_device_extension()`: 添加设备扩展。
            *   `create_device()`: 创建 Vulkan 逻辑设备。
            *   `check_device_extensions()`: 检查设备扩展是否可用。
            *   `add_callback_create_device()`: 添加设备创建回调函数。
            *   `add_callback_destory_device()`: 添加设备销毁回调函数。
*   **成员变量**:
    *   `physical_device`: 物理设备句柄。
    *   `device`: 逻辑设备句柄。
    *   `queue_graphics`, `queue_presentation`, `queue_compute`: 队列句柄。

### `VulkanBase/components/VulkanInstance.h`

*   **概述**: 封装了 Vulkan 实例的创建和管理，包括实例层、扩展的添加和检查，以及调试信使的设置。
*   **关键类**:
    *   `VulkanInstance`:
        *   **关键函数**:
            *   `get_instance()`: 获取 Vulkan 实例句柄。
            *   `add_instance_layer()`: 添加实例层。
            *   `add_instance_extension()`: 添加实例扩展。
            *   `create_instance()`: 创建 Vulkan 实例。
            *   `check_instance_layers()`: 检查实例层是否可用。
            *   `check_instance_extensions()`: 检查实例扩展是否可用。
            *   `use_latest_api_version()`: 使用最新的 Vulkan API 版本。
            *   `create_debug_messenger()`: 创建调试信使（仅在 DEBUG 模式下启用）。
*   **成员变量**:
    *   `instance`: Vulkan 实例句柄。
    *   `instance_layers`: 实例层列表。
    *   `instance_extensions`: 实例扩展列表。
    *   `debug_messenger`: 调试信使句柄。
    *   `surface`: 表面句柄。

### `VulkanBase/components/VulkanOperation.h`

*   **概述**: 提供了 Vulkan 图像操作的静态函数，如缓冲区到图像的复制和图像之间的 Blit 操作，并处理图像内存屏障。
*   **关键结构体**:
    *   `image_operation`:
        *   **关键静态函数**:
            *   `cmd_copy_buffer_to_image()`: 将缓冲区数据复制到图像。
            *   `cmd_blit_image()`: 在图像之间执行 Blit 操作（缩放、格式转换）。
        *   **辅助结构体**: `image_memory_barrier_parameter_pack` 用于简化内存屏障参数。

### `VulkanBase/components/VulkanPipepline.h`

*   **概述**: 封装了 Vulkan 管线布局和图形/计算管线的创建。
*   **关键类**:
    *   `VulkanPipelineLayout`: 封装 `VkPipelineLayout`，提供管线布局的创建。
        *   **关键函数**: `create()`。
    *   `VulkanPipeline`: 封装 `VkPipeline`，提供图形管线和计算管线的创建。
        *   **关键函数**: `create()` (重载用于图形和计算管线)。
*   **辅助结构体**:
    *   `GraphicsPipelineCreateInfoPack`: 辅助结构体，用于组织图形管线创建所需的所有信息，包括着色器阶段、顶点输入、输入汇编、视口、裁剪、光栅化、多重采样、深度/模板测试、颜色混合和动态状态。

### `VulkanBase/components/VulkanRenderPassWithFramebuffers.h`

*   **概述**: 封装了 Vulkan 渲染通道和帧缓冲区的创建和管理。
*   **关键类**:
    *   `VulkanRenderPass`: 封装 `VkRenderPass`，提供渲染通道的创建、开始、结束和子通道切换命令。
        *   **关键函数**: `create()`, `cmd_begin()`, `cmd_next()`, `cmd_end()`, `clear()`。
    *   `VulkanFramebuffer`: 封装 `VkFramebuffer`，提供帧缓冲区的创建。
        *   **关键函数**: `create()`, `clear()`。
*   **辅助结构体**:
    *   `RenderPassWithFramebuffers`: 组合 `VulkanRenderPass` 和 `std::vector<VulkanFramebuffer>`，方便管理一组渲染通道和其对应的帧缓冲区。

### `VulkanBase/components/VulkanShaderModule.h`

*   **概述**: 封装了 Vulkan 着色器模块的创建，支持从文件路径或二进制代码创建。
*   **关键类**:
    *   `VulkanShaderModule`:
        *   **关键函数**:
            *   `create()` (重载用于 `VkShaderModuleCreateInfo`, 文件路径, 或二进制代码)。
            *   `stage_create_info()`: 生成 `VkPipelineShaderStageCreateInfo`，用于管线创建。

### `VulkanBase/components/VulkanSync.h`

*   **概述**: 封装了 Vulkan 同步原语，包括栅栏（Fence）和信号量（Semaphore），用于 GPU 和 CPU 之间的同步以及 GPU 内部的同步。
*   **关键类**:
    *   `fence`: 封装 `VkFence`，提供栅栏的创建、等待、重置和状态查询。
        *   **关键函数**: `create()`, `wait()`, `reset()`, `wait_and_reset()`, `status()`。
    *   `semaphore`: 封装 `VkSemaphore`，提供信号量的创建。
        *   **关键函数**: `create()`。

## 继承关系

*   `VulkanBufferMemory` 继承自 `VulkanBuffer` 和 `VulkanDeviceMemory` (多重继承)。
*   `VulkanImageMemory` 继承自 `VulkanImage` 和 `VulkanDeviceMemory` (多重继承)。
*   `VulkanDeviceLocalBuffer` 继承自 `VulkanBufferMemory`。
*   `VulkanVertexBuffer` 继承自 `VulkanDeviceLocalBuffer`。
*   `VulkanIndexBuffer` 继承自 `VulkanDeviceLocalBuffer`。
*   `VulkanUniformBuffer` 继承自 `VulkanDeviceLocalBuffer`。

## 其他有助于理解项目结构的信息

*   **单例模式**: 项目中大量使用了单例模式（例如 `VulkanAppLauncher`, `VulkanCore`, `VulkanExecutionManager`, `VulkanPipelineManager`, `VulkanSwapchainManager`, `VulkanCommand`），这表明这些类在整个应用程序生命周期中只有一个实例，用于集中管理和访问核心 Vulkan 资源。
*   **RAII (Resource Acquisition Is Initialization)**: 许多 Vulkan 句柄（如 `VkDeviceMemory`, `VkBuffer`, `VkImage`, `VkPipelineLayout` 等）都被封装在 C++ 类中，并在构造函数中创建，在析构函数中销毁。这遵循了 RAII 原则，确保资源被正确管理和释放，避免内存泄漏和 Vulkan 资源泄露。
*   **模块化设计**: 项目将 Vulkan 的各个方面（如设备管理、交换链、命令、缓冲区、管线、描述符、同步）分解为独立的类和组件，提高了代码的可维护性和可扩展性。
*   **错误处理**: 项目中使用了自定义的 `result_t` 类型来封装 `VkResult`，并结合 `std::format` 进行错误日志输出，便于调试和错误追踪。
*   **回调机制**: `VulkanDevice` 和 `VulkanSwapchainManager` 中使用了回调函数 (`std::function<void()>`) 来处理设备创建/销毁和交换链创建/销毁时的额外操作，这增加了系统的灵活性。
*   **着色器**: 项目中包含 `.shader` 和 `.spv` 文件，表明使用了 SPIR-V 格式的预编译着色器。
*   **图像加载**: 使用 `stb_image.h` 库进行图像加载，这是一个常用的单文件图像加载库。
*   **GLM**: 使用 GLM 库进行数学运算，特别是矩阵变换，这在图形编程中非常常见。
