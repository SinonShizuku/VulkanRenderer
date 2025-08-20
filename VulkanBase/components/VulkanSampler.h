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
