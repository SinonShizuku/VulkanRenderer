#pragma once
#include <queue>

#include "../VKStart.h"
#include "VulkanInstance.h"

class VulkanDevice {
public:
    VulkanDevice(VulkanInstance* instance) {
        this->instance = instance;
    }

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

    void set_device_extensions(const std::vector<const char*>& extensionNames) {
        device_extensions = extensionNames;
    }

    void add_device_extension(const char *extension_name);

    VkResult acquire_physical_devices() {
        uint32_t device_count = 0;
        if (VkResult result = vkEnumeratePhysicalDevices(instance->get_instance(), &device_count, nullptr); result != VK_SUCCESS) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!device_count) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to find any physical device supports vulkan!\n");
            abort();
        }
        available_physical_devices.resize(device_count);
        VkResult result = vkEnumeratePhysicalDevices(instance->get_instance(), &device_count, available_physical_devices.data());
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", int32_t(result));
        }
        return result;
    }

    VkResult get_queue_family_indices(VkPhysicalDevice physical_device, bool enable_graphics_queue,bool enable_compute_queue, uint32_t (&queue_family_indices)[3]) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        if (!queue_family_count) return VK_RESULT_MAX_ENUM;
        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

        auto&[ig,ip,ic] = queue_family_indices;
        ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
        for (uint32_t i = 0; i < queue_family_count; i++) {
            // 这三个VkBool32变量指示是否可获取（指应该被获取且能获取）相应队列族索引
            VkBool32
                support_graphics = enable_graphics_queue && queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
                support_presentation = false,
                support_compute = enable_compute_queue && queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
            // 只在创建了window surface时获取支持呈现的队列族的索引
            if (instance->get_surface())
                if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, instance->get_surface(), &support_presentation)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to determine if the queue family supports presentation!\nError code: {}\n", int32_t(result));
                    return result;
                }
            // 同时支持图形操作与计算
            if (support_graphics && support_presentation) {
                if (support_presentation) {
                    ig = ip = ic = i;
                    break;
                }
                if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED)
                    ig = ic = i;
                if (!instance->get_surface()) break;
            }
            // 若任何一个队列族索引可以被取得但尚未被取得，将其值覆写为i
            if (support_graphics && ig == VK_QUEUE_FAMILY_IGNORED)
                ig = i;
            if (support_presentation && ip == VK_QUEUE_FAMILY_IGNORED)
                ip = i;
            if (support_compute && ic == VK_QUEUE_FAMILY_IGNORED)
                ic = i;
        }
        // 若任何需要被取得的队列族索引尚未被取得，则函数执行失败
        if (ig == VK_QUEUE_FAMILY_IGNORED && enable_graphics_queue ||
            ip == VK_QUEUE_FAMILY_IGNORED && instance->get_surface()||
            ic == VK_QUEUE_FAMILY_IGNORED && enable_compute_queue ) {
            return VK_RESULT_MAX_ENUM;
        }

        // 成功
        queue_family_index_graphics = ig;
        queue_family_index_presentation = ip;
        queue_family_index_compute = ic;
        return VK_SUCCESS;
    }

    VkResult determine_physical_device(uint32_t device_index = 0, bool enable_graphics_queue = true, bool enable_compute_queue = true) {
        static constexpr uint32_t  not_found = INT32_MAX;
        struct queue_family_index_combination {
            uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
            uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
            uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
        };
        static std::vector<queue_family_index_combination> queue_family_index_combinations(available_physical_devices.size());
        auto&[ig,ip,ic] = queue_family_index_combinations[device_index];

        if (ig == not_found && enable_graphics_queue ||
            ip == not_found && instance->get_surface()||
            ic == not_found && enable_compute_queue ) {
            return VK_RESULT_MAX_ENUM;
        }

        if (ig == VK_QUEUE_FAMILY_IGNORED && enable_graphics_queue ||
            ip == VK_QUEUE_FAMILY_IGNORED && instance->get_surface()||
            ic == VK_QUEUE_FAMILY_IGNORED && enable_compute_queue ) {
            uint32_t indices[3];
            VkResult result = get_queue_family_indices(available_physical_devices[device_index], enable_graphics_queue,enable_compute_queue, indices);
            if (result == VK_SUCCESS || result == VK_RESULT_MAX_ENUM) {
                if (enable_graphics_queue) ig = indices[0] & INT32_MAX;
                if (instance->get_surface()) ip = indices[1] & INT32_MAX;
                if (enable_compute_queue) ic = indices[2] & INT32_MAX;
            }
            if (result) return result;
        }
        else {
            queue_family_index_graphics = enable_graphics_queue?ig:VK_QUEUE_FAMILY_IGNORED;
            queue_family_index_presentation = instance->get_surface()?ip:VK_QUEUE_FAMILY_IGNORED;
            queue_family_index_compute = enable_compute_queue?ic:VK_QUEUE_FAMILY_IGNORED;
        }
        physical_device = available_physical_devices[device_index];
        return VK_SUCCESS;
    }

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
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a vulkan logical device!\nError code: {}\n", int32_t(result));
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
        std::cout << std::format("Renderer: {}\n", physical_device_properties.deviceName);
        // to be continued
        return VK_SUCCESS;
    }

    //以下函数用于创建逻辑设备失败后
    VkResult check_device_extensions(std::span<const char*> extensions_to_check, const char* layer_name = nullptr) const {
        uint32_t extension_count;
        std::vector<VkExtensionProperties> available_extensions;
        if (VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &extension_count, nullptr)) {
            layer_name ?
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of device extensions!\nLayer name:{}\n", layer_name) :
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of device extensions!\n");
            return result;
        }
        if (extension_count) {
            available_extensions.resize(extension_count);
            if (VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &extension_count, available_extensions.data())) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate device extension properties!\nError code: {}\n", int32_t(result));
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

private:
    VulkanInstance *instance;
    VkSurfaceKHR *surface;

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

};


