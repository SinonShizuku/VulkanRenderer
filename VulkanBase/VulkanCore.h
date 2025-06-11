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
        vulkan_swapchain.clear_swapchain_create_info();
        vulkan_instance.set_debug_messenger(VK_NULL_HANDLE);
    }

    static VulkanCore& get_singleton() {
        static VulkanCore* singleton = new VulkanCore();
        return *singleton;
    }

    VkResult wait_idle() const {
        VkResult result = vkDeviceWaitIdle(vulkan_device.get_device());
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

    [[nodiscard]] VulkanSwapchain & get_vulkan_swapchain() {
        return vulkan_swapchain;
    }

    // device function in VulkanCore
    VkResult acquire_physical_devices() {
        uint32_t device_count = 0;
        if (VkResult result = vkEnumeratePhysicalDevices(vulkan_instance.get_instance(), &device_count, nullptr); result != VK_SUCCESS) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!device_count) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to find any physical device supports vulkan!\n");
            abort();
        }
        std::vector<VkPhysicalDevice>& available_physical_devices = vulkan_device.get_available_physical_devices();
        available_physical_devices.resize(device_count);
        VkResult result = vkEnumeratePhysicalDevices(vulkan_instance.get_instance(), &device_count, available_physical_devices.data());
        if (result != VK_SUCCESS) {
            outstream << std::format("[ VulkanDevice ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", int32_t(result));
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
            if (auto &surface = vulkan_instance.get_surface()) {
                if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &support_presentation)) {
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

    VkResult determine_physical_device(uint32_t device_index = 0, bool enable_graphics_queue = true, bool enable_compute_queue = true) {
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
            VkResult result = get_queue_family_indices(available_physical_devices[device_index], enable_graphics_queue,enable_compute_queue, indices);
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
            vulkan_swapchain.clear_swapchain_create_info();
        }
        for (auto& i : vulkan_device.get_callbacks_destroy_device())
            i();
        if (auto device = vulkan_device.get_device())//防函数执行失败后再次调用时发生重复销毁
            vkDestroyDevice(device, nullptr),
            vulkan_device.set_device(VK_NULL_HANDLE);
        return vulkan_device.create_device(flags);
    }

    // swapchain function in VulkanCore
    VkResult set_surface_formats(VkSurfaceFormatKHR surface_format, VkSwapchainCreateInfoKHR &swapchain_create_info) {
        bool format_is_available = false;
        auto &available_surface_formats = vulkan_swapchain.get_available_surface_formats();
        if (!surface_format.format) {
            for (auto &i : available_surface_formats) {
                if (i.colorSpace == surface_format.colorSpace) {
                    swapchain_create_info.imageFormat = i.format;
                    swapchain_create_info.imageColorSpace = i.colorSpace;
                    format_is_available = true;
                    break;
                }
            }
        }
        else {
            for (auto &i : available_surface_formats) {
                if (i.colorSpace == surface_format.colorSpace && i.format == surface_format.format) {
                    swapchain_create_info.imageFormat = i.format;
                    swapchain_create_info.imageColorSpace = i.colorSpace;
                    format_is_available = true;
                    break;
                }
            }
        }
        if (!format_is_available)
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        if (vulkan_swapchain.get_swapchain())
            return recreate_swapchain();
        return VK_SUCCESS;
    }

    VkResult get_surface_formats() {
        uint32_t surfaceFormatCount;
        if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_device.get_physical_device(), vulkan_instance.get_surface(), &surfaceFormatCount, nullptr)) {
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to get the count of surface formats!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!surfaceFormatCount)
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to find any supported surface format!\n"),
            abort();
        auto &available_surface_formats = vulkan_swapchain.get_available_surface_formats();
        available_surface_formats.resize(surfaceFormatCount);
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_device.get_physical_device(), vulkan_instance.get_surface(), &surfaceFormatCount, available_surface_formats.data());
        if (result)
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to get surface formats!\nError code: {}\n", int32_t(result));
        return result;
    }

    VkResult create_swapchain(bool limit_frame_rate = true, VkSwapchainCreateFlagsKHR flags = 0) {
        auto &available_surface_formats = vulkan_swapchain.get_available_surface_formats();
        auto &swapchain_create_info = vulkan_swapchain.get_swapchain_create_info();
        VkSurfaceCapabilitiesKHR surface_capabilities = {};

        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_device.get_physical_device(), vulkan_instance.get_surface(), &surface_capabilities); result != VK_SUCCESS) {
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
            return result;
        }
        swapchain_create_info.minImageCount = surface_capabilities.minImageCount + (surface_capabilities.maxImageCount > surface_capabilities.minImageCount);
        swapchain_create_info.imageExtent =
            surface_capabilities.currentExtent.width == -1 ?
            VkExtent2D{
                glm::clamp(default_window_size.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
                glm::clamp(default_window_size.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height)}:
            surface_capabilities.currentExtent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.preTransform = surface_capabilities.currentTransform;
        if (surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
            swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        else
            for (size_t i=0;i<4;i++) {
                if (surface_capabilities.supportedCompositeAlpha & 1 << i) {
                    swapchain_create_info.compositeAlpha = VkCompositeAlphaFlagBitsKHR(surface_capabilities.supportedCompositeAlpha & 1 << i);
                    break;
                }
            }
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        else
            outstream << std::format("[ VulkanSwapchain ] WARNING\nVK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported!\n");

        if (available_surface_formats.empty())
            if (VkResult result = get_surface_formats())
                return result;

        if (!swapchain_create_info.imageFormat) {
            if (set_surface_formats({VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},swapchain_create_info) &&
                set_surface_formats({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},swapchain_create_info)) {
                swapchain_create_info.imageFormat = available_surface_formats[0].format;
                swapchain_create_info.imageColorSpace = available_surface_formats[0].colorSpace;
                outstream << std::format("[ VulkanSwapchain ] WARNING\nFailed to select a four-component UNORM surface format!\n");
            }
        }

        uint32_t surface_present_mode_count = 0;
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_device.get_physical_device(), vulkan_instance.get_surface(), &surface_present_mode_count, nullptr)) {
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to get the count of surface present modes!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!surface_present_mode_count)
            outstream << std::format("[ graphicsBase ] ERROR\nFailed to find any surface present mode!\n"),
            abort();
        std::vector<VkPresentModeKHR> surface_present_modes(surface_present_mode_count);
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_device.get_physical_device(), vulkan_instance.get_surface(), &surface_present_mode_count, surface_present_modes.data())) {
            outstream << std::format("[ graphicsBase ] ERROR\nFailed to get surface present modes!\nError code: {}\n", int32_t(result));
            return result;
        }
        swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!limit_frame_rate)
            for (size_t i = 0; i < surface_present_mode_count; i++)
                if (surface_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                    swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }

        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.flags = flags;
        swapchain_create_info.surface = vulkan_instance.get_surface();
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.clipped = VK_TRUE;

        if (VkResult result = vulkan_swapchain.create_swapchain_internal())
            return result;

        for (auto&i:vulkan_swapchain.get_callbacks_create_swapchain())
            i();

        return VK_SUCCESS;
    }

    VkResult recreate_swapchain() {
        VkSurfaceCapabilitiesKHR surface_capabilities = {};
        auto &swapchain_create_info = vulkan_swapchain.get_swapchain_create_info();

        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_device.get_physical_device(), vulkan_instance.get_surface(), &surface_capabilities)) {
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (surface_capabilities.currentExtent.width == 0 ||
            surface_capabilities.currentExtent.height == 0)
            return VK_SUBOPTIMAL_KHR;
        swapchain_create_info.imageExtent = surface_capabilities.currentExtent;

        swapchain_create_info.oldSwapchain = vulkan_swapchain.get_swapchain();

        VkResult result = vkQueueWaitIdle(vulkan_device.get_queue_graphics());
        if (!result && vulkan_device.get_queue_graphics()!= vulkan_device.get_queue_presentation())
            result = vkQueueWaitIdle(vulkan_device.get_queue_presentation());
        if (result) {
            outstream << std::format("[ VulkanSwapchain ] ERROR\nFailed to wait for the queue to be idle!\nError code: {}\n", int32_t(result));
            return result;
        }

        for (auto&i:vulkan_swapchain.get_callbacks_destroy_swapchain())
            i();
        // 销毁旧ImageViews
        for (auto&i:vulkan_swapchain.get_swapchain_image_views())
            if (i) vkDestroyImageView(vulkan_device.get_device(), i, nullptr);

        vulkan_swapchain.clear_swapchain_image_views();
        for (auto&i:vulkan_swapchain.get_callbacks_create_swapchain())
            i();
        return VK_SUCCESS;
    }




private:
    static VulkanCore* singleton;
    VulkanInstance vulkan_instance;
    VulkanDevice vulkan_device = VulkanDevice();
    VulkanSwapchain vulkan_swapchain = VulkanSwapchain(&vulkan_device);
};
