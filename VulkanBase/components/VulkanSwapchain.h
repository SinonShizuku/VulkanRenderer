//
// Created by Jhong on 2025/6/1.
//

#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H
#include "../VKStart.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"


class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanInstance *instance,VulkanDevice *device){
        vulkan_instance = instance;
        vulkan_device = device;
    }
    // getter
    [[nodiscard]] const VkFormat &get_available_surface_format(uint32_t index) const {
        return available_surface_formats[index].format;
    }

    [[nodiscard]] const VkColorSpaceKHR &get_available_surface_color_space(uint32_t index) const {
        return available_surface_formats[index].colorSpace;
    }

    [[nodiscard]] uint32_t get_available_surface_format_count() const {
        return uint32_t(available_surface_formats.size());
    }

    [[nodiscard]] VkSwapchainKHR get_swapchain() const {
        return swapchain;
    }

    [[nodiscard]] VkImage get_swapchain_image(uint32_t index) const{
        return swapchain_images[index];
    }

    [[nodiscard]] VkImageView get_swapchain_image_view(uint32_t index) const {
        return swapchain_image_views[index];
    }

    [[nodiscard]] uint32_t get_swapchain_image_count() const {
        return uint32_t(swapchain_images.size());
    }

    [[nodiscard]] VkSwapchainCreateInfoKHR& get_swapchain_create_info(){
        return swapchain_create_info;
    }

    [[nodiscard]] std::vector<void(*)()> & get_callbacks_create_swapchain() {
        return callbacks_create_swapchain;
    }

    [[nodiscard]] std::vector<void(*)()> & get_callbacks_destroy_swapchain() {
        return callbacks_destroy_swapchain;
    }

    [[nodiscard]] std::vector<VkImageView> & get_swapchain_image_views() {
        return swapchain_image_views;
    }

    // setter
    void set_swapchain(VkSwapchainKHR swapchain) {
        this->swapchain = swapchain;
    }

    void set_swapchain_create_info(const VkSwapchainCreateInfoKHR &swapchain_create_info) {
        this->swapchain_create_info = swapchain_create_info;
    }

    void clear_swapchain_images() {
        swapchain_images.clear();
    }

    VkResult set_surface_formats(VkSurfaceFormatKHR surface_format) {
        bool format_is_available = false;
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
        if (swapchain)
            return recreate_swapchain();
        return VK_SUCCESS;
    }

    VkResult get_surface_formats() {
        uint32_t surfaceFormatCount;
        if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_device->get_physical_device(), vulkan_instance->get_surface(), &surfaceFormatCount, nullptr)) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to get the count of surface formats!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!surfaceFormatCount)
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to find any supported surface format!\n"),
            abort();
        available_surface_formats.resize(surfaceFormatCount);
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_device->get_physical_device(), vulkan_instance->get_surface(), &surfaceFormatCount, available_surface_formats.data());
        if (result)
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to get surface formats!\nError code: {}\n", int32_t(result));
        return result;
    }

    VkResult create_swapchain(bool limit_frame_rate = true, VkSwapchainCreateFlagsKHR flags = 0) {
        VkSurfaceCapabilitiesKHR surface_capabilities = {};
        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_device->get_physical_device(), vulkan_instance->get_surface(), &surface_capabilities); result != VK_SUCCESS) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
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
            std::cout << std::format("[ VulkanSwapchain ] WARNING\nVK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported!\n");

        if (available_surface_formats.empty())
            if (VkResult result = get_surface_formats())
                return result;

        if (!swapchain_create_info.imageFormat) {
            if (set_surface_formats({VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}) &&
                set_surface_formats({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})) {
                swapchain_create_info.imageFormat = available_surface_formats[0].format;
                swapchain_create_info.imageColorSpace = available_surface_formats[0].colorSpace;
                std::cout << std::format("[ VulkanSwapchain ] WARNING\nFailed to select a four-component UNORM surface format!\n");
            }
        }

        uint32_t surface_present_mode_count = 0;
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_device->get_physical_device(), vulkan_instance->get_surface(), &surface_present_mode_count, nullptr)) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to get the count of surface present modes!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!surface_present_mode_count)
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to find any surface present mode!\n"),
            abort();
        std::vector<VkPresentModeKHR> surfacePresentModes(surface_present_mode_count);
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_device->get_physical_device(), vulkan_instance->get_surface(), &surface_present_mode_count, surfacePresentModes.data())) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get surface present modes!\nError code: {}\n", int32_t(result));
            return result;
        }
        swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!limit_frame_rate)
            for (size_t i = 0; i < surface_present_mode_count; i++)
                if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                    swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }

        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.flags = flags;
        swapchain_create_info.surface = vulkan_instance->get_surface();
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.clipped = VK_TRUE;

        if (VkResult result = create_swapchain_internal())
            return result;

        for (auto&i:callbacks_create_swapchain)
            i();

        return VK_SUCCESS;
    }

    VkResult recreate_swapchain() {
        VkSurfaceCapabilitiesKHR surface_capabilities = {};
        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_device->get_physical_device(), vulkan_instance->get_surface(), &surface_capabilities)) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (surface_capabilities.currentExtent.width == 0 ||
            surface_capabilities.currentExtent.height == 0)
            return VK_SUBOPTIMAL_KHR;
        swapchain_create_info.imageExtent = surface_capabilities.currentExtent;

        swapchain_create_info.oldSwapchain = swapchain;

        VkResult result = vkQueueWaitIdle(vulkan_device->get_queue_graphics());
        if (!result && vulkan_device->get_queue_graphics()!= vulkan_device->get_queue_presentation())
            result = vkQueueWaitIdle(vulkan_device->get_queue_presentation());
        if (result) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to wait for the queue to be idle!\nError code: {}\n", int32_t(result));
            return result;
        }

        for (auto&i:callbacks_destroy_swapchain)
            i();
        // 销毁旧ImageViews
        for (auto&i:swapchain_image_views)
            if (i) vkDestroyImageView(vulkan_device->get_device(), i, nullptr);
        swapchain_image_views.clear();
        for (auto&i:callbacks_create_swapchain)
            i();
        return VK_SUCCESS;
    }

    void add_callback_create_swapchain(void(*function)()) {
        callbacks_create_swapchain.push_back(function);
    }

    void add_callback_destroy_swapchain(void(*function)()) {
        callbacks_destroy_swapchain.push_back(function);
    }

private:
    VulkanInstance *vulkan_instance;
    VulkanDevice *vulkan_device;

    std::vector<VkSurfaceFormatKHR> available_surface_formats;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    VkSwapchainCreateInfoKHR swapchain_create_info = {};

    std::vector<void(*)()> callbacks_create_swapchain;
    std::vector<void(*)()> callbacks_destroy_swapchain;

    VkResult create_swapchain_internal() {
        if (VkResult result = vkCreateSwapchainKHR(vulkan_device->get_device(), &swapchain_create_info, nullptr, &swapchain)) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to create a swapchain!\nError code: {}\n", int32_t(result));
            return result;
        }

        // 获取交换链图像
        uint32_t swapchain_image_count = 0;
        if (VkResult result = vkGetSwapchainImagesKHR(vulkan_device->get_device(), swapchain, &swapchain_image_count, nullptr)) {
            std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to get the count of swapchain images!\nError code: {}\n", int32_t(result));
            return result;
        }
        swapchain_images.resize(swapchain_image_count);
        if (VkResult result = vkGetSwapchainImagesKHR(vulkan_device->get_device(), swapchain, &swapchain_image_count, swapchain_images.data())) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get swapchain images!\nError code: {}\n", int32_t(result));
            return result;
        }

        // 创建image view (图像的使用方式)
        swapchain_image_views.resize(swapchain_image_count);
        VkImageViewCreateInfo swapchain_image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain_create_info.imageFormat,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        };
        for (size_t i = 0; i < swapchain_image_count; i++) {
            swapchain_image_view_create_info.image = swapchain_images[i];
            if (VkResult result = vkCreateImageView(vulkan_device->get_device(), &swapchain_image_view_create_info, nullptr, &swapchain_image_views[i]); result != VK_SUCCESS) {
                std::cout << std::format("[ VulkanSwapchain ] ERROR\nFailed to create a swapchain image view!\nError code: {}\n", int32_t(result));
                return result;
            }
        }
        return VK_SUCCESS;
    }
};



#endif //VULKAN_SWAPCHAIN_H
