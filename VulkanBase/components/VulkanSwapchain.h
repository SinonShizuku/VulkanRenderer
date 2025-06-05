//
// Created by Jhong on 2025/6/1.
//

#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H
#include <utility>

#include "../VKStart.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"


class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanDevice *device){
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

    [[nodiscard]] std::vector<VkSurfaceFormatKHR> & get_available_surface_formats() {
        return available_surface_formats;
    }

    // setter
    void set_swapchain(VkSwapchainKHR swapchain) {
        this->swapchain = swapchain;
    }


    void clear_swapchain_images() {
        swapchain_images.clear();
    }

    void clear_swapchain_image_views() {
        swapchain_image_views.clear();
    }

    void clear_swapchain_create_info() {
        swapchain_create_info = {};
    }

    void add_callback_create_swapchain(void(*function)()) {
        callbacks_create_swapchain.push_back(function);
    }

    void add_callback_destroy_swapchain(void(*function)()) {
        callbacks_destroy_swapchain.push_back(function);
    }

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

private:
    VulkanDevice *vulkan_device;

    std::vector<VkSurfaceFormatKHR> available_surface_formats;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    VkSwapchainCreateInfoKHR swapchain_create_info = {};

    std::vector<void(*)()> callbacks_create_swapchain;
    std::vector<void(*)()> callbacks_destroy_swapchain;

};



#endif //VULKAN_SWAPCHAIN_H
