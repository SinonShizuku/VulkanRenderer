//
// Created by Jhong on 2025/5/31.
//

#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H
#include "../VKStart.h"



class VulkanInstance {
public:
    // getter
    [[nodiscard]] VkInstance get_instance() const {
        return instance;
    }

    [[nodiscard]] std::vector<const char *> &get_instance_layers() {
        return instance_layers;
    }

    [[nodiscard]] std::vector<const char *> &get_instance_extensions() {
        return instance_extensions;
    }

    [[nodiscard]] VkSurfaceKHR &get_surface() {
        return surface;
    }

    [[nodiscard]] VkDebugUtilsMessengerEXT get_debug_messenger() const {
        return debug_messenger;
    }


    // setter
    void set_instance_layers(const std::vector<const char *> &instance_layers) {
        this->instance_layers = instance_layers;
    }

    void set_instance_extensions(const std::vector<const char *> &instance_extensions) {
        this->instance_extensions = instance_extensions;
    }

    void set_surface(VkSurfaceKHR surface) {
        this->surface = surface;
    }

    void set_instance(VkInstance instance) {
        this->instance = instance;
    }

    void set_debug_messenger(VkDebugUtilsMessengerEXT debug_messenger) {
        this->debug_messenger = debug_messenger;
    }


    void add_instance_layer(const char *layerName);

    void add_instance_extension(const char *extensionName);

    VkResult create_instance(VkInstanceCreateFlags flags = 0) {
#ifndef NDEBUG
        add_instance_layer("VK_LAYER_KHRONOS_validation");
        add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        VkApplicationInfo app_info{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = api_version
        };
        VkInstanceCreateInfo instance_info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags = flags,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<uint32_t>(instance_layers.size()),
            .ppEnabledLayerNames = instance_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size()),
            .ppEnabledExtensionNames = instance_extensions.data()
        };
        if (VkResult result = vkCreateInstance(&instance_info, nullptr, &instance); result != VK_SUCCESS) {
            std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to create a vulkan instance!\nError code: {}\n", int32_t(result));
            return result;
        }
        std::cout << std::format(
        "Vulkan API Version: {}.{}.{}\n",
        VK_VERSION_MAJOR(api_version),
        VK_VERSION_MINOR(api_version),
        VK_VERSION_PATCH(api_version));
#ifndef NDEBUG
        create_debug_messenger();
#endif
        return VK_SUCCESS;
    }

    VkResult check_instance_layers(std::span<const char *> layers_to_check) {
        uint32_t layer_count = 0;
        std::vector<VkLayerProperties> available_layers;
        if (VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr); result != VK_SUCCESS) {
            std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to get the count of instance layers!\n");
            return result;
        }
        if (layer_count) {
            available_layers.resize(layer_count);
            if (VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()); result != VK_SUCCESS) {
                std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to enumerate instance layer properties!\nError code: {}\n", int32_t(result));
                return result;
            }
            for (auto& i : layers_to_check) {
                bool found = false;
                for (auto& j : available_layers)
                    if (!strcmp(i, j.layerName)) {
                        found = true;
                        break;
                    }
                if (!found)
                    i = nullptr;
            }
        }
        else {
            for (auto& i : layers_to_check) {
                i = nullptr;
            }
        }
        return VK_SUCCESS;
    }

    VkResult check_instance_extensions(std::span<const char *> extensions_to_check,
                                       const char *layer_name = nullptr) const {
        uint32_t extension_count = 0;
        std::vector<VkExtensionProperties> available_extensions;
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, nullptr); result != VK_SUCCESS) {
            layer_name?
            std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to get the count of instance extensions!\nLayer name:{}\n", layer_name) :
            std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to get the count of instance extensions!\n");
            return result;
        }
        if (extension_count) {
            available_extensions.resize(extension_count);
            if (VkResult result = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, available_extensions.data()); result != VK_SUCCESS) {
                std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to enumerate instance extension properties!\nError code: {}\n", int32_t(result));
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
                    i = nullptr;
            }
        }
        else {
            for (auto& i : extensions_to_check) {
                i = nullptr;
            }
        }
        return VK_SUCCESS;
    }

    uint32_t get_api_version() {
        return api_version;
    }

    VkResult use_latest_api_version() {
        if (vkGetInstanceProcAddr(VK_NULL_HANDLE,"vkEnumerateInstanceVersion"))
            return vkEnumerateInstanceVersion(&api_version);
        return VK_SUCCESS;
    }



private:
    VkInstance instance;
    std::vector<const char *> instance_layers;
    std::vector<const char*> instance_extensions;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    uint32_t api_version = VK_API_VERSION_1_0;



    VkResult create_debug_messenger() {
        static PFN_vkDebugUtilsMessengerCallbackEXT debug_messenger_callback = [](
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* user_data) -> VkBool32 {
            std::cout << std::format("{}\n\n", callback_data->pMessage);
            return VK_FALSE;
        };
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_messenger_callback
        };
        auto vk_create_debug_utils_messenger =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (vk_create_debug_utils_messenger) {
            VkResult result = vk_create_debug_utils_messenger(instance,&debug_messenger_create_info,nullptr,&debug_messenger);
            if (result != VK_SUCCESS) {
                std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to create a debug messenger!\nError code: {}\n", int32_t(result));
            }
            return result;
        }
        std::cout << std::format("[ VulkanInstanceModule ] ERROR\nFailed to get the function pointer of vkCreateDebugUtilsMessengerEXT!\n");
        return VK_RESULT_MAX_ENUM;
    }


};



#endif //VULKANINSTANCE_H
