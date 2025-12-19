#pragma once
#include "../../Start.h"
#include "../VulkanCore.h"

class VulkanSampler {
    VkSampler handle = VK_NULL_HANDLE;
public:
    VulkanSampler() = default;
    VulkanSampler(VkSamplerCreateInfo &create_info) {
        create(create_info);
    }
    VulkanSampler(VulkanSampler &&other) noexcept {MoveHandle;}
    ~VulkanSampler() {DestroyHandleBy(VulkanCore::get_singleton().get_vulkan_device().get_device(),vkDestroySampler);}

    // getter
    DefineHandleTypeOperator;
    DefineAddressFunction;

    // non-const function
    result_t create(VkSamplerCreateInfo &create_info) {
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        VkResult result = vkCreateSampler(VulkanCore::get_singleton().get_vulkan_device().get_device(), &create_info, nullptr, &handle);
        if (result)
            outstream << std::format("[ VulkanSampler ] ERROR\nFailed to create a sampler!\nError code: {}\n", int32_t(result));
        return result;
    }
};

class VulkanDepthSampler : public VulkanSampler {
public:
    VulkanDepthSampler() {
        VkSamplerCreateInfo create_info = {};
        VkFilter shadow_map_filter = VulkanCore::get_singleton().get_vulkan_device().format_is_filterable(VK_FORMAT_D16_UNORM, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        create_info.magFilter = shadow_map_filter;
        create_info.minFilter = shadow_map_filter;
        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        // create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_info.addressModeV = create_info.addressModeU;
        create_info.addressModeW = create_info.addressModeU;
        create_info.mipLodBias = 0.0f;
        create_info.maxAnisotropy = 1.0f;
        create_info.minLod = 0.0f;
        create_info.maxLod = 1.0f;
        create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        create(create_info);
    }

};
