#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <fmt/core.h>

#include "vk_mem_alloc.h"

#include "Core.hpp"
#include "Buffer.hpp"

namespace vkc {

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};

struct ImageCreateInfo {
    uint32_t width;
    uint32_t height;
    uint32_t depth = 1;
    VkExtent3D extent = {0, 0, 1};
    VkFormat format;
    VkImageTiling image_tiling;
    VkImageUsageFlags usage_flags;
    VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    VkMemoryPropertyFlags required_flags;
};

struct ImageViewCreateInfo {
    VkImage image;
    VkFormat format;
    VkImageAspectFlags aspect_flags;
};

void create_image(vkc::Core& core, const ImageCreateInfo& image_create_info, AllocatedImage& image);

void create_image_view(vkc::Core& core, const ImageViewCreateInfo& image_view_create_info, VkImageView& image_view);

void copyBufferToImage(vkc::Core& core, VkCommandPool command_pool, vkc::AllocatedBuffer buffer, vkc::AllocatedImage image, uint32_t width, uint32_t height);

void transitionImageLayout(vkc::Core& core, VkCommandPool command_pool, vkc::AllocatedImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

}

#endif // IMAGE_HPP