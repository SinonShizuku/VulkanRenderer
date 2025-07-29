#pragma once
#include "../VKStart.h"
#include "../VulkanCore.h"

class VulkanCommandBuffer {
    friend class VulkanCommandPool;
    VkCommandBuffer handle = VK_NULL_HANDLE;
public:
    VulkanCommandBuffer() = default;
    VulkanCommandBuffer(VulkanCommandBuffer &&other) noexcept {MoveHandle};

    // getter
    DefineHandleTypeOperator;
    DefineAddressFunction;

    // const function
    result_t begin(VkCommandBufferUsageFlags usage_flags, VkCommandBufferInheritanceInfo &inheritance_info) const {
        inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = usage_flags,
            .pInheritanceInfo = &inheritance_info
        };
        VkResult result = vkBeginCommandBuffer(handle, &begin_info);
        if (result)
            outstream << std::format("[ VulkanCommandBuffer ] ERROR\nFailed to begin a command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }

    result_t begin(VkCommandBufferUsageFlags usage_flags = 0) const {
        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = usage_flags
        };
        VkResult result = vkBeginCommandBuffer(handle, &begin_info);
        if (result)
            outstream << std::format("[ VulkanCommandBuffer ] ERROR\nFailed to begin a command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }

    result_t end() const {
        VkResult result = vkEndCommandBuffer(handle);
        if (result)
            outstream << std::format("[ VulkanCommandBuffer ] ERROR\nFailed to end a command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }
};

class VulkanCommandPool {
    VkCommandPool handle = VK_NULL_HANDLE;
public:
    VulkanCommandPool() = default;
    VulkanCommandPool(VkCommandPoolCreateInfo& createInfo) {
        create(createInfo);
    }
    VulkanCommandPool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0) {
        create(queue_family_index, flags);
    }
    VulkanCommandPool(VulkanCommandPool&& other) noexcept { MoveHandle; }
    ~VulkanCommandPool() { DestroyHandleBy(VulkanCore::get_singleton().get_vulkan_device().get_device(),vkDestroyCommandPool); }
    //Getter
    DefineHandleTypeOperator;
    DefineAddressFunction;
    //Const Function
    result_t allocate_buffers(array_ref<VkCommandBuffer> buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
        VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = handle,
            .level = level,
            .commandBufferCount = uint32_t(buffers.Count())
        };
        VkResult result = vkAllocateCommandBuffers(VulkanCore::get_singleton().get_vulkan_device().get_device(), &allocateInfo, buffers.Pointer());
        if (result)
            outstream << std::format("[ commandPool ] ERROR\nFailed to allocate command buffers!\nError code: {}\n", int32_t(result));
        return result;
    }
    result_t allocate_buffers(array_ref<VulkanCommandBuffer> buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
        return allocate_buffers(
            { &buffers[0].handle, buffers.Count() },
            level);
    }
    void free_buffers(array_ref<VkCommandBuffer> buffers) const {
        vkFreeCommandBuffers(VulkanCore::get_singleton().get_vulkan_device().get_device(), handle, buffers.Count(), buffers.Pointer());
        memset(buffers.Pointer(), 0, buffers.Count() * sizeof(VkCommandBuffer));
    }
    void free_buffers(array_ref<VulkanCommandBuffer> buffers) const {
        free_buffers({ &buffers[0].handle, buffers.Count() });
    }
    //Non-const Function
    result_t create(VkCommandPoolCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        VkResult result = vkCreateCommandPool(VulkanCore::get_singleton().get_vulkan_device().get_device(), &createInfo, nullptr, &handle);
        if (result)
            outstream << std::format("[ commandPool ] ERROR\nFailed to create a command pool!\nError code: {}\n", int32_t(result));
        return result;
    }
    result_t create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0) {
        VkCommandPoolCreateInfo createInfo = {
            .flags = flags,
            .queueFamilyIndex = queueFamilyIndex
        };
        return create(createInfo);
    }
};
