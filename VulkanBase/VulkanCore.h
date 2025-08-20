#pragma once
#include "../Start.h"
#include "components/VulkanDevice.h"
#include "components/VulkanInstance.h"

static void add_layer_or_extension(std::vector<const char*>& container, const char* name) {
    for (auto& i : container)
        if (!strcmp(name, i))
            return;
    container.push_back(name);
}

inline void VulkanInstance::add_instance_layer(const char *layerName) {
    add_layer_or_extension(instance_layers, layerName);
}

inline void VulkanInstance::add_instance_extension(const char *extensionName) {
    add_layer_or_extension(instance_extensions, extensionName);
}

inline void VulkanDevice::add_device_extension(const char *extension_name) {
    add_layer_or_extension(device_extensions, extension_name);
}


class VulkanCore {
public:
    VulkanCore(){};
    ~VulkanCore(){};

    void destroy_singleton() {
        auto instance = vulkan_instance.get_instance();
        if (!instance) {
            return;
        }
        if (auto device = vulkan_device.get_device()) {
            wait_idle();
            for (auto &i:vulkan_device.get_callbacks_destroy_device())
                i();
            vkDestroyDevice(device, nullptr);
        }
        if (auto surface = vulkan_instance.get_surface())
            vkDestroySurfaceKHR(instance, surface, nullptr);
        if (auto debug_messenger = vulkan_instance.get_debug_messenger()) {
            PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger =
                reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (destroy_debug_messenger) destroy_debug_messenger(instance , debug_messenger, nullptr);
        }
        vkDestroyInstance(instance, nullptr);

        // 将句柄置空
        vulkan_instance.set_instance(VK_NULL_HANDLE);
        vulkan_device.set_physical_device(VK_NULL_HANDLE);
        vulkan_device.set_device(VK_NULL_HANDLE);
        vulkan_instance.set_surface(VK_NULL_HANDLE);
        vulkan_instance.set_debug_messenger(VK_NULL_HANDLE);
    }

    static VulkanCore& get_singleton() {
        static VulkanCore singleton = VulkanCore();
        return singleton;
    }

    result_t wait_idle() const {
        result_t result = vkDeviceWaitIdle(vulkan_device.get_device());
        if (result)
            outstream << std::format("[ VulkanCore ] ERROR\nFailed to wait for the device to be idle!\nError code: {}\n", int32_t(result));
        return result;

    }

    // getter
    [[nodiscard]] VulkanInstance & get_vulkan_instance() {
        return vulkan_instance;
    }


    [[nodiscard]] VulkanDevice & get_vulkan_device() {
        return vulkan_device;
    }

    // device function in VulkanCore
    result_t acquire_physical_devices() {
        uint32_t device_count = 0;
        if (result_t result = vkEnumeratePhysicalDevices(vulkan_instance.get_instance(), &device_count, nullptr); result != VK_SUCCESS) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!device_count) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to find any physical device supports vulkan!\n");
            abort();
        }
        std::vector<VkPhysicalDevice>& available_physical_devices = vulkan_device.get_available_physical_devices();
        available_physical_devices.resize(device_count);
        result_t result = vkEnumeratePhysicalDevices(vulkan_instance.get_instance(), &device_count, available_physical_devices.data());
        if (result != VK_SUCCESS) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", int32_t(result));
        }
        return result;
    }

    result_t get_queue_family_indices(VkPhysicalDevice physical_device, bool enable_graphics_queue,bool enable_compute_queue, uint32_t (&queue_family_indices)[3]) {
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
            if (auto &surface = vulkan_instance.get_surface()) {
                if (result_t result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &support_presentation)) {
                    outstream << std::format("[ VulkanDevice ] ERROR\nFailed to determine if the queue family supports presentation!\nError code: {}\n", int32_t(result));
                    return result;
                }
            }
            // 同时支持图形操作与计算
            if (support_graphics && support_presentation) {
                if (support_presentation) {
                    ig = ip = ic = i;
                    break;
                }
                if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED)
                    ig = ic = i;
                if (!vulkan_instance.get_surface()) break;
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
            ip == VK_QUEUE_FAMILY_IGNORED && vulkan_instance.get_surface()||
            ic == VK_QUEUE_FAMILY_IGNORED && enable_compute_queue ) {
            return VK_RESULT_MAX_ENUM;
        }

        // 成功
        vulkan_device.set_queue_family_index_graphics(ig);
        vulkan_device.set_queue_family_index_compute(ic);
        vulkan_device.set_queue_family_index_presentation(ip);
        return VK_SUCCESS;
    }

    result_t determine_physical_device(uint32_t device_index = 0, bool enable_graphics_queue = true, bool enable_compute_queue = true) {
        static constexpr uint32_t  not_found = INT32_MAX;
        auto& available_physical_devices = vulkan_device.get_available_physical_devices();
        struct queue_family_index_combination {
            uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
            uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
            uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
        };
        static std::vector<queue_family_index_combination> queue_family_index_combinations(available_physical_devices.size());
        auto&[ig,ip,ic] = queue_family_index_combinations[device_index];

        if (ig == not_found && enable_graphics_queue ||
            ip == not_found && vulkan_instance.get_surface()||
            ic == not_found && enable_compute_queue ) {
            return VK_RESULT_MAX_ENUM;
        }

        if (ig == VK_QUEUE_FAMILY_IGNORED && enable_graphics_queue ||
            ip == VK_QUEUE_FAMILY_IGNORED && vulkan_instance.get_surface()||
            ic == VK_QUEUE_FAMILY_IGNORED && enable_compute_queue ) {
            uint32_t indices[3];
            result_t result = get_queue_family_indices(available_physical_devices[device_index], enable_graphics_queue,enable_compute_queue, indices);
            if (result == VK_SUCCESS || result == VK_RESULT_MAX_ENUM) {
                if (enable_graphics_queue) ig = indices[0] & INT32_MAX;
                if (vulkan_instance.get_surface()) ip = indices[1] & INT32_MAX;
                if (enable_compute_queue) ic = indices[2] & INT32_MAX;
            }
            if (result) return result;
        }
        else {
            vulkan_device.set_queue_family_index_graphics(enable_graphics_queue?ig:VK_QUEUE_FAMILY_IGNORED);
            vulkan_device.set_queue_family_index_presentation(vulkan_instance.get_surface()?ip:VK_QUEUE_FAMILY_IGNORED);
            vulkan_device.set_queue_family_index_compute(enable_compute_queue?ic:VK_QUEUE_FAMILY_IGNORED);
        }
        vulkan_device.set_physical_device(available_physical_devices[device_index]);
        return VK_SUCCESS;
    }


private:
    VulkanInstance vulkan_instance;
    VulkanDevice vulkan_device;
};
