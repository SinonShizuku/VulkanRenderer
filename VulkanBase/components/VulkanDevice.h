#pragma once
#include <queue>

#include "../VKStart.h"
#include "VulkanInstance.h"

class VulkanDevice {
public:
    // getter
    [[nodiscard]] VkDevice get_device() const {
        return device;
    }

    [[nodiscard]] std::vector<const char *> get_device_extensions() const {
        return device_extensions;
    }

    [[nodiscard]] VkPhysicalDevice get_physical_device() const {
        return physical_device;
    }

    [[nodiscard]] VkPhysicalDeviceProperties get_physical_device_properties() const {
        return physical_device_properties;
    }

    [[nodiscard]] VkPhysicalDeviceMemoryProperties get_physical_device_memory_properties() const {
        return physical_device_memory_properties;
    }

    [[nodiscard]] std::vector<VkPhysicalDevice> & get_available_physical_devices() {
        return available_physical_devices;
    }

    [[nodiscard]] VkPhysicalDevice get_available_physical_device(uint32_t index) const {
        return available_physical_devices[index];
    }

    [[nodiscard]] uint32_t get_available_physical_device_count() const {
        return available_physical_devices.size();
    }

    [[nodiscard]] uint32_t get_queue_family_index_graphics() const {
        return queue_family_index_graphics;
    }

    [[nodiscard]] uint32_t get_queue_family_index_presentation() const {
        return queue_family_index_presentation;
    }

    [[nodiscard]] uint32_t get_queue_family_index_compute() const {
        return queue_family_index_compute;
    }

    [[nodiscard]] VkQueue get_queue_graphics() const {
        return queue_graphics;
    }

    [[nodiscard]] VkQueue get_queue_presentation() const {
        return queue_presentation;
    }

    [[nodiscard]] VkQueue get_queue_compute() const {
        return queue_compute;
    }

    [[nodiscard]] std::vector<void(*)()> & get_callbacks_create_device() {
        return callbacks_create_device;
    }

    [[nodiscard]] std::vector<void(*)()> & get_callbacks_destroy_device() {
        return callbacks_destroy_device;
    }

    void set_device(VkDevice device) {
        this->device = device;
    }

    void set_queue_family_index_graphics(uint32_t queue_family_index_graphics) {
        this->queue_family_index_graphics = queue_family_index_graphics;
    }

    void set_queue_family_index_presentation(uint32_t queue_family_index_presentation) {
        this->queue_family_index_presentation = queue_family_index_presentation;
    }

    void set_queue_family_index_compute(uint32_t queue_family_index_compute) {
        this->queue_family_index_compute = queue_family_index_compute;
    }

    void set_device_extensions(const std::vector<const char*>& extensionNames) {
        device_extensions = extensionNames;
    }

    void set_physical_device(VkPhysicalDevice physical_device) {
        this->physical_device = physical_device;
    }

    void add_device_extension(const char *extension_name);

    VkResult create_device(VkDeviceCreateFlags flags = 0) {
        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info[3] = {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority},
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority},
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority}
        };
        uint32_t queue_create_info_count = 0;
        if (queue_family_index_graphics!=VK_QUEUE_FAMILY_IGNORED)
            queue_create_info[queue_create_info_count++].queueFamilyIndex = queue_family_index_graphics;
        if (queue_family_index_presentation!=VK_QUEUE_FAMILY_IGNORED
            && queue_family_index_presentation != queue_family_index_graphics)
            queue_create_info[queue_create_info_count++].queueFamilyIndex = queue_family_index_presentation;
        if (queue_family_index_compute!=VK_QUEUE_FAMILY_IGNORED
            && queue_family_index_compute != queue_family_index_graphics
            && queue_family_index_compute != queue_family_index_presentation)
            queue_create_info[queue_create_info_count++].queueFamilyIndex = queue_family_index_compute;

        VkPhysicalDeviceFeatures physical_device_features;
        vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);
        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .flags = flags,
            .queueCreateInfoCount = queue_create_info_count,
            .pQueueCreateInfos = queue_create_info,
            .enabledExtensionCount = uint32_t(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
            .pEnabledFeatures = &physical_device_features
        };
        if (VkResult result = vkCreateDevice(physical_device, &device_create_info, nullptr, &device);result!=VK_SUCCESS) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to create a vulkan logical device!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (queue_family_index_graphics!=VK_QUEUE_FAMILY_IGNORED)
            vkGetDeviceQueue(device,queue_family_index_graphics,0,&queue_graphics);
        if (queue_family_index_presentation!=VK_QUEUE_FAMILY_IGNORED)
            vkGetDeviceQueue(device,queue_family_index_presentation,0,&queue_presentation);
        if (queue_family_index_compute!=VK_QUEUE_FAMILY_IGNORED)
            vkGetDeviceQueue(device,queue_family_index_compute,0,&queue_compute);
        vkGetPhysicalDeviceProperties(physical_device,&physical_device_properties);
        vkGetPhysicalDeviceMemoryProperties(physical_device,&physical_device_memory_properties);
        // 输出所用的物理设备名称
        outstream << std::format("Renderer: {}\n", physical_device_properties.deviceName);
        for (auto& i : callbacks_create_device)
            i();
        return VK_SUCCESS;
    }

    //以下函数用于创建逻辑设备失败后
    VkResult check_device_extensions(std::span<const char*> extensions_to_check, const char* layer_name = nullptr) const {
        uint32_t extension_count;
        std::vector<VkExtensionProperties> available_extensions;
        if (VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &extension_count, nullptr)) {
            layer_name ?
                outstream << std::format("[ VulkanDevice ] ERROR\nFailed to get the count of device extensions!\nLayer name:{}\n", layer_name) :
                outstream << std::format("[ VulkanDevice ] ERROR\nFailed to get the count of device extensions!\n");
            return result;
        }
        if (extension_count) {
            available_extensions.resize(extension_count);
            if (VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &extension_count, available_extensions.data())) {
                outstream << std::format("[ VulkanDevice ] ERROR\nFailed to enumerate device extension properties!\nError code: {}\n", int32_t(result));
                return result;
            }
            for (auto& i : extensions_to_check) {
                bool found = false;
                for (auto& j : available_extensions)
                    if (!strcmp(i, j.extensionName)) {
                        found = true;
                        break;
                    }
                if (!found)
                    i = nullptr;//If a required extension isn't available, set it to nullptr
            }
        }
        else
            for (auto& i : extensions_to_check)
                i = nullptr;
        return VK_SUCCESS;
    }

    void add_callback_create_device(void(*function)()) {
        callbacks_create_device.push_back(function);
    }

    void add_callback_destory_device(void(*function)()) {
        callbacks_destroy_device.push_back(function);
    }


private:

    VkPhysicalDevice physical_device{};
    VkPhysicalDeviceProperties physical_device_properties{};
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties{};
    std::vector<VkPhysicalDevice> available_physical_devices;

    uint32_t queue_family_index_graphics = VK_QUEUE_FAMILY_IGNORED;
    uint32_t queue_family_index_presentation = VK_QUEUE_FAMILY_IGNORED;
    uint32_t queue_family_index_compute = VK_QUEUE_FAMILY_IGNORED;
    VkQueue queue_graphics;
    VkQueue queue_presentation;
    VkQueue queue_compute;

    VkDevice device;
    std::vector<const char*> device_extensions;

    std::vector<void(*)()> callbacks_create_device;
    std::vector<void(*)()> callbacks_destroy_device;


};
