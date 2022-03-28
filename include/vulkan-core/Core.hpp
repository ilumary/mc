#ifndef CORE_HPP
#define CORE_HPP

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Window.hpp"

namespace vkc {


class Core {
    VkInstance instance_{};
    VkDebugUtilsMessengerEXT debug_messenger_{};
    VkSurfaceKHR surface_{};
    VkPhysicalDevice physical_device_{};
    VkDevice device_{};
    VkQueue graphics_queue_{};
    VkQueue present_queue_{};
    uint32_t graphics_queue_family_index_ = 0;

    VmaAllocator allocator_{};

public:
    explicit Core(Window& window);
    ~Core();

    inline VkInstance instance() noexcept { return instance_; }

    inline VkDebugUtilsMessengerEXT debug_messenger() noexcept { return debug_messenger_; }

    inline VkSurfaceKHR surface() noexcept { return surface_; }

    inline VkPhysicalDevice physical_device() noexcept { return physical_device_; }

    inline VkDevice device() noexcept { return device_; }

    inline VkQueue graphics_queue() noexcept { return graphics_queue_; }

    inline VkQueue present_queue() noexcept { return present_queue_; }

    inline uint32_t graphics_queue_family_index() noexcept { return graphics_queue_family_index_; }

    inline VmaAllocator allocator() noexcept { return allocator_; }
};

}; // end namespace vkc

#endif // CORE_HPP
