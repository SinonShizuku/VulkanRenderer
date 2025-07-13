#pragma once
#include "../VKStart.h"
#include "../VulkanCore.h"

class fence {
    VkFence handle = VK_NULL_HANDLE;
public:
    fence(VkFenceCreateInfo& createInfo) {
        create(createInfo);
    }
    // default constructor
    fence(VkFenceCreateFlags flags = 0) {
        create(flags);
    }
    fence(fence&& other) noexcept { MoveHandle; }
    ~fence() { DestroyHandleBy(VulkanCore::get_singleton().get_vulkan_device().get_device(),vkDestroyFence); }
    // Getter
    DefineHandleTypeOperator;
    DefineAddressFunction;
    // Const Function
    result_t wait() const {
        VkResult result = vkWaitForFences(VulkanCore::get_singleton().get_vulkan_device().get_device(), 1, &handle, false, UINT64_MAX);
        if (result)
            outstream << std::format("[ fence ] ERROR\nFailed to wait for the fence!\nError code: {}\n", int32_t(result));
        return result;
    }
    result_t reset() const {
        VkResult result = vkResetFences(VulkanCore::get_singleton().get_vulkan_device().get_device(), 1, &handle);
        if (result)
            outstream << std::format("[ fence ] ERROR\nFailed to reset the fence!\nError code: {}\n", int32_t(result));
        return result;
    }

    result_t wait_and_reset() const {
        VkResult result = wait();
        result || ((result = reset()));
        return result;
    }
    result_t status() const {
        VkResult result = vkGetFenceStatus(VulkanCore::get_singleton().get_vulkan_device().get_device(), handle);
        if (result < 0) //vkGetFenceStatus(...)成功时有两种结果，所以不能仅仅判断result是否非0
            outstream << std::format("[ fence ] ERROR\nFailed to get the status of the fence!\nError code: {}\n", int32_t(result));
        return result;
    }
    //Non-const Function
    result_t create(VkFenceCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult result = vkCreateFence(VulkanCore::get_singleton().get_vulkan_device().get_device(), &createInfo, nullptr, &handle);
        if (result)
            outstream << std::format("[ fence ] ERROR\nFailed to create a fence!\nError code: {}\n", int32_t(result));
        return result;
    }
    result_t create(VkFenceCreateFlags flags = 0) {
        VkFenceCreateInfo createInfo = {
            .flags = flags
        };
        return create(createInfo);
    }
};

class semaphore {
    VkSemaphore handle = VK_NULL_HANDLE;
public:
    //semaphore() = default;
    semaphore(VkSemaphoreCreateInfo& createInfo) {
        create(createInfo);
    }
    //默认构造器创建未置位的信号量
    semaphore(/*VkSemaphoreCreateFlags flags*/) {
        create();
    }
    semaphore(semaphore&& other) noexcept { MoveHandle; }
    ~semaphore() { DestroyHandleBy(VulkanCore::get_singleton().get_vulkan_device().get_device(),vkDestroySemaphore); }
    //Getter
    DefineHandleTypeOperator;
    DefineAddressFunction;
    //Non-const Function
    result_t create(VkSemaphoreCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(VulkanCore::get_singleton().get_vulkan_device().get_device(), &createInfo, nullptr, &handle);
        if (result)
            outstream << std::format("[ semaphore ] ERROR\nFailed to create a semaphore!\nError code: {}\n", int32_t(result));
        return result;
    }
    result_t create(/*VkSemaphoreCreateFlags flags*/) {
        VkSemaphoreCreateInfo createInfo = {};
        return create(createInfo);
    }
};
