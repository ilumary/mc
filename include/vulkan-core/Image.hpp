#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <fmt/core.h>

#include "vk_mem_alloc.h"

#include "Core.hpp"

namespace vkc {

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};

struct ImageCreateInfo {
    uint32_t tex_width;
    uint32_t tex_height;
    VkFormat format;
    VkImageTiling image_tiling;
    VkImageUsageFlags usage_flags;
    //VkMemoryPropertyFlags memory_properties;
};

struct ImageViewCreateInfo {
    VkImage image;
    VkFormat format;
    VkImageAspectFlags aspect_flags;
};

void create_image(vkc::Core& core, const ImageCreateInfo& image_create_info, AllocatedImage& image);
void create_image_view(vkc::Core& core, const ImageViewCreateInfo& image_view_create_info, VkImageView& image_view);

}

#endif // IMAGE_HPP