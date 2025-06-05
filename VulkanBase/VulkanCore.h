#pragma once
#include "VKStart.h"
#include "components/VulkanDevice.h"
#include "components/VulkanInstance.h"
#include "components/VulkanSwapchain.h"

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
    VulkanCore() {

    }
    ~VulkanCore() {
        auto instance = vulkan_instance.get_instance();
        if (!instance) {
            return;
        }
        if (auto device = vulkan_device.get_device()) {
            wait_idle();
            if (auto swapchain = vulkan_swapchain.get_swapchain()) {
                for (auto& i : vulkan_swapchain.get_callbacks_destroy_swapchain())
                    i();
                for (auto& i : vulkan_swapchain.get_swapchain_image_views())
                    if (i)
                        vkDestroyImageView(device, i, nullptr);
                vkDestroySwapchainKHR(device, swapchain, nullptr);
            }
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
        vulkan_swapchain.set_swapchain(VK_NULL_HANDLE);
        vulkan_swapchain.clear_swapchain_images();
        vulkan_swapchain.get_swapchain_image_views().resize(0);
        vulkan_swapchain.set_swapchain_create_info({});
        vulkan_instance.set_debug_messenger(VK_NULL_HANDLE);
    }

    static VulkanCore& get_singleton() {
        static VulkanCore* singleton = new VulkanCore();
        return *singleton;
    }

    VkResult wait_idle() const {
        VkResult result = vkDeviceWaitIdle(vulkan_device.get_device());
        if (result)
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to wait for the device to be idle!\nError code: {}\n", int32_t(result));
        return result;
    }

    // getter
    [[nodiscard]] VulkanInstance & get_vulkan_instance() {
        return vulkan_instance;
    }


    [[nodiscard]] VulkanDevice & get_vulkan_device() {
        return vulkan_device;
    }

    [[nodiscard]] VulkanSwapchain & get_vulkan_swapchain() {
        return vulkan_swapchain;
    }

    // device in core
    VkResult recreate_device(VkDeviceCreateFlags flags = 0) {
        if (VkResult result = wait_idle()) {
            return result;
        }
        if (vulkan_swapchain.get_swapchain()) {
            for (auto &i: vulkan_swapchain.get_callbacks_destroy_swapchain())
                i();
            auto swapchain_image_views = vulkan_swapchain.get_swapchain_image_views();
            for (auto &i:swapchain_image_views)
                if (i) vkDestroyImageView(vulkan_device.get_device(), i, nullptr);
            swapchain_image_views.resize(0);
            vkDestroySwapchainKHR(vulkan_device.get_device(), vulkan_swapchain.get_swapchain(), nullptr);
            vulkan_swapchain.set_swapchain(VK_NULL_HANDLE);
            vulkan_swapchain.set_swapchain_create_info({});
        }
        for (auto& i : vulkan_device.get_callbacks_destroy_device())
            i();
        if (auto device = vulkan_device.get_device())//防函数执行失败后再次调用时发生重复销毁
            vkDestroyDevice(device, nullptr),
            vulkan_device.set_device(VK_NULL_HANDLE);
        return vulkan_device.create_device(flags);
    }


private:
    static VulkanCore* singleton;
    VulkanInstance vulkan_instance;
    VulkanDevice vulkan_device = VulkanDevice(&vulkan_instance);
    VulkanSwapchain vulkan_swapchain = VulkanSwapchain(&vulkan_instance,&vulkan_device);
};
