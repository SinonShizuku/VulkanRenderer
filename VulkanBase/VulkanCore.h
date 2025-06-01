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
    VulkanCore() = default;
    static VulkanCore& get_singleton() {
        static VulkanCore* singleton = new VulkanCore();
        return *singleton;
    }

    // getter
    [[nodiscard]] VulkanInstance & get_vulkan_instance() {
        return vulkan_instance;
    }


    [[nodiscard]] VulkanDevice & get_vulkan_device() {
        return vulkan_device;
    }


private:
    static VulkanCore* singleton;
    VulkanInstance vulkan_instance;
    VulkanDevice vulkan_device = VulkanDevice(&vulkan_instance);
};

