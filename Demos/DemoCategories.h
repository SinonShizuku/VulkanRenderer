#pragma once
#include "../Start.h"

enum class DemoCategoryType {
    VULKAN_TESTS,           // Vulkan功能测试
    BASIC_IMPLEMENTATION,   // 基础实现
    RASTERIZATION,         // 光栅化
    PBR_RAY_TRACING,       // PBR光线追踪
    // 预留扩展空间
    CUSTOM_CATEGORY_1,
    CUSTOM_CATEGORY_2
};

using DemoType = std::string;

inline std::unordered_map<std::string,std::vector<DemoType>> demos = {
    // Vulkan Tests
    {"VULKAN_TESTS",
        {
        "BuffersAndPictureTest",
        "ImagelessFramebufferTest",
        "DynamicRenderingTest",
        "OffScreenRenderingTest",
            "DepthAttachmentTest",
            "DeferredRenderingTest"
        }
    }

};

inline std::string current_demo_name = "BuffersAndPictureTest";

