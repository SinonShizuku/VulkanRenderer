cmake_minimum_required(VERSION 3.28)
project(VulkanRenderer)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 23)


# glfw & glm
include_directories(External/glfw/include)
include_directories(External/glm)
include_directories(External/tinygltf/)


# Vulkan
if(WIN32)
    # change to your VulkanSDK include path
    set(VULKAN_SDK_PATH "D:/VulkanSDK/1.4.313.1")
    include_directories(${VULKAN_SDK_PATH}/Include)
endif ()

# imgui
set(IMGUI_DIR "External/imgui")
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)
file(GLOB IMGUI_CORE_SOURCES
        "${IMGUI_DIR}/*.cpp"
)
set(IMGUI_BACKEND_SOURCES
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

add_executable(VulkanRenderer main.cpp
        Start.h
        Launcher/VulkanAppLauncher.cpp
        Launcher/VulkanAppLauncher.h
        VulkanBase/VulkanCore.h
        VulkanBase/components/VulkanInstance.h
        VulkanBase/components/VulkanDevice.h
        VulkanBase/components/VulkanSync.h
        VulkanBase/VulkanSwapchainManager.h
        VulkanBase/VulkanExecutionManager.h
        VulkanBase/components/VulkanCommand.h
        VulkanBase/VulkanPipelineManager.h
        VulkanBase/components/VulkanRenderPassWithFramebuffers.h
        VulkanBase/components/VulkanPipepline.h
        Demos/SharedResourceManager.h
        VulkanBase/components/VulkanShaderModule.h
        VulkanBase/components/VulkanMemory.h
        Geometry/Vertex.h
        VulkanBase/components/VulkanDescriptor.h
        Interaction/Texture.h
        VulkanBase/VulkanContext.h
        VulkanBase/components/VulkanOperation.h
        stb_image_implementation.cpp
        VulkanBase/components/VulkanSampler.h
        VulkanBase/components/VulkanTexture.h
        Demos/DemoManager.h
        UI/ImGuiManager.h
        Demos/DemoCategories.h
        Demos/DemoBase.h
        Demos/VulkanTests/BuffersAndPictureTest.h
        Demos/VulkanTests/ImagelessFramebufferTest.h
        Demos/VulkanTests/DynamicRenderingTest.h
        Demos/VulkanTests/OffScreenRenderingTest.h
        Demos/VulkanTests/DepthAttachmentTest.h
        Demos/VulkanTests/DeferredRenderingTest.h
        Shader/ShaderLoader.h
        Shader/ShaderLoader.cpp
        Geometry/Model.h
        Demos/BasicRendering/glTFLoading.h
        Interaction/Camera.h
        Demos/DemoBase3D.h
        Demos/BasicRendering/ShadowMapping.h)

 target_compile_definitions(VulkanRenderer PRIVATE 
    PROJECT_ROOT_PATH="${CMAKE_SOURCE_DIR}"
)

target_sources(VulkanRenderer PRIVATE
        ${IMGUI_CORE_SOURCES}
        ${IMGUI_BACKEND_SOURCES}
)

target_link_directories(VulkanRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/External/lib)

if (WIN32)
    target_link_libraries(VulkanRenderer 
                            ${CMAKE_CURRENT_SOURCE_DIR}/External/glfw/lib-vc2022/glfw3.lib ${VULKAN_SDK_PATH}/Lib/vulkan-1.lib)
else()
    # Linux/WSL specific linking
    target_link_libraries(VulkanRenderer glfw vulkan)
endif()