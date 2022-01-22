#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include "Core.hpp"

namespace vkc {

class Swapchain {

    VkDevice device_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> images_{};
    std::vector<VkImageView> image_views_{};
    VkFormat image_format_ = VK_FORMAT_UNDEFINED;

public:

    Swapchain(Core& core, const VkExtent2D& extent);
    ~Swapchain();

    inline const VkSwapchainKHR swapchain() const { return swapchain_; }

    inline std::vector<VkImage> images() const { return images_; }

    inline std::vector<VkImageView> image_views() const { return image_views_; }

    inline VkFormat format() const { return image_format_; }

};

} // namespace vkc

#endif // SWAPCHAIN_HPP