#include "Swapchain.hpp"

#include <VkBootstrap.h>

namespace vkc {
    Swapchain::Swapchain(Core& core, const VkExtent2D& extent) : device_{core.device()} {
        vkb::SwapchainBuilder swapchain_builder{core.physical_device(), core.device(), core.surface()};

        vkb::Swapchain vkb_swapchain = swapchain_builder
                                        .use_default_format_selection()
                                        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                        .set_desired_extent(extent.width, extent.height)
                                        .build()
                                        .value();
        
        swapchain_ = vkb_swapchain.swapchain;
        images_ = vkb_swapchain.get_images().value();
        image_views_ = vkb_swapchain.get_image_views().value();
        image_format_ = vkb_swapchain.image_format;
    }

    Swapchain::~Swapchain() {
        if (device_) {
            for (VkImageView image_view : image_views_) {
                vkDestroyImageView(device_, image_view, nullptr);
            }
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        }
    }
}