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

enum class DemoType {
    // Vulkan功能测试
    BuffersAndPictureTest,
    VULKAN_DEVICE_TEST,
    VULKAN_BUFFER_TEST,
    VULKAN_TEXTURE_TEST,

    // 基础实现
    FIRST_TRIANGLE,
    INDEXED_TRIANGLE,
    COLORED_TRIANGLE,
    UNIFORM_BUFFER_TRIANGLE,

    // 光栅化
    WIREFRAME_RENDERING,
    TEXTURE_MAPPING,
    PHONG_LIGHTING,
    SHADOW_MAPPING,

    // PBR光线追踪
    BASIC_RAY_TRACING,
    PBR_MATERIALS,
    GLOBAL_ILLUMINATION,
    DENOISING,

    // 预留扩展
    CUSTOM_SCENE_START = 1000
};

struct DemoInfo {
    DemoType type;
    DemoCategoryType category;
    std::string name;
    std::string description;
    bool is_implemented = false;
};