cmake_minimum_required(VERSION 3.28)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 23)

add_executable(VulkanRenderer main.cpp
        VulkanBase/VKStart.h
        VulkanAppLauncher.cpp
        VulkanAppLauncher.h
        VulkanBase/VulkanCore.h
        VulkanBase/components/VulkanInstance.h
        VulkanBase/components/VulkanDevice.h
        VulkanBase/components/VulkanSwapchain.h)

target_link_libraries(VulkanRenderer glfw vulkan)
