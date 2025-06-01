//
// Created by Jhong on 2025/6/1.
//

#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H
#include "../VKStart.h"

class VulkanSwapchain {
public:
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

    VkResult get_surface_formats() {

    }

    VkResult set_surface_formats(VkSurfaceFormatKHR surfaceFormat) {

    }

    VkResult create_swapchain(bool limit_frame_rate = true, VkSwapchainCreateFlagsKHR = 0) {}

    VkResult recreate_swapchain() {}

private:
    std::vector<VkSurfaceFormatKHR> available_surface_formats;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};

    VkResult create_swapchain_internal() {

    }
};



#endif //VULKAN_SWAPCHAIN_H
