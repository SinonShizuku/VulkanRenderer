#pragma once
#include "../VulkanBase/VKFormat.h"
#include <stb/stb_image.h>

class Texture {
protected:
    static std::unique_ptr<uint8_t[]> load_file_internal(const auto* address, size_t file_size, VkExtent2D &extent, VulkanFormatInfo format_info) {
        if constexpr (ENABLE_DEBUG_MESSENGER) {
            if (!(format_info.rawDataType == VulkanFormatInfo::floatingPoint && format_info.sizePerComponent == 4) &&
    !(format_info.rawDataType == VulkanFormatInfo::integer && between_closed<int32_t>(1, format_info.sizePerComponent, 2)))
                outstream << std::format("[ Texture ] ERROR\nRequired format is not available for source image data!\n"),
                abort();
        }
        int& width = reinterpret_cast<int&>(extent.width);
        int& height = reinterpret_cast<int&>(extent.height);
        int channel_count;
        void* p_image_data = nullptr;
        if constexpr (std::same_as<decltype(address), const char*>) {
            if (format_info.rawDataType == VulkanFormatInfo::integer) {
                if (format_info.sizePerComponent == 1)
                    p_image_data =  stbi_load(address, &width, &height, &channel_count, format_info.componentCount);
                else
                    p_image_data = stbi_load_16(address, &width, &height, &channel_count, format_info.componentCount);
            }
            else
                p_image_data = stbi_loadf(address, &width, &height, &channel_count, format_info.componentCount);
            if (!p_image_data)
                outstream << std::format("[ Texture ] ERROR\nFailed to load the file: {}\n", address);
        }
        if constexpr (std::same_as<decltype(address), const uint8_t*>) {
            if (file_size > INT32_MAX) {
                outstream << std::format("[ Texture ] ERROR\nFailed to load image data from the given address! Data size must be less than 2G!\n");
                return {};
            }
            if (format_info.rawDataType == VulkanFormatInfo::integer) {
                if (format_info.sizePerComponent == 1)
                    p_image_data = stbi_load_from_memory(address, file_size, &width, &height, &channel_count, format_info.componentCount);
                else
                    p_image_data = stbi_load_16_from_memory(address, file_size, &width, &height, &channel_count, format_info.componentCount);
            }
            else
                p_image_data = stbi_loadf_from_memory(address, file_size, &width, &height, &channel_count, format_info.componentCount);
            if (!p_image_data)
                outstream << std::format("[ Texture ] ERROR\nFailed to load image data from the given address!\n");
        }
        return std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(p_image_data));
    }
public:
    static std::unique_ptr<uint8_t[]> load_file(const char* file_path, VkExtent2D &extent, VulkanFormatInfo format_info) {
        return load_file_internal(file_path, 0, extent, format_info);
    }
    static std::unique_ptr<uint8_t[]> load_file(const uint8_t* file_binaries, size_t file_size, VkExtent2D &extent, VulkanFormatInfo format_info) {
        return load_file_internal(file_binaries, file_size, extent, format_info);
    }
};
