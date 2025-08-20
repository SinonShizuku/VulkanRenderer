cmake_minimum_required(VERSION 3.28)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 23)

if(WIN32)
    include_directories(D:/cpplib/glfw/include)
    include_directories(D:/VulkanSDK/1.4.313.1/Include)
    include_directories(D:/cpplib/glm)
endif ()

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
        VulkanBase/VulkanResourceManager.h
        VulkanBase/components/VulkanShaderModule.h
        VulkanBase/components/VulkanBuffers.h
        Scene/Vertex.h
        VulkanBase/components/VulkanDescriptor.h
        Scene/Texture.h
        VulkanBase/VulkanContext.h
        VulkanBase/components/VulkanOperation.h
        stb_image_implementation.cpp
        VulkanBase/components/VulkanSampler.h
        VulkanBase/components/VulkanTexture.h
)

if (WIN32)
target_link_libraries(VulkanRenderer D:/cpplib/glfw/lib-mingw-w64/libglfw3.a D:/VulkanSDK/1.4.313.1/Lib/vulkan-1.lib)
else()
    # Linux/WSL specific linking
    target_link_libraries(VulkanRenderer glfw vulkan)
endif()



