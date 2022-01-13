#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VkBootstrap.h"

#include <fmt/core.h>

#include <iostream>
#include <vector>

class Application {

    GLFWwindow* window_ = nullptr;
    VkExtent2D window_extent_{};
    VkInstance instance_{};
    VkPhysicalDevice physical_device_{};
    VkSurfaceKHR surface_{};
    VkDevice device_{};
    VkQueue graphics_queue_{};
    uint32_t graphics_queue_family_index_ = 0;
    VkQueue present_queue_{};
    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;
    VkDebugUtilsMessengerEXT debug_messenger_{};
    
    VkSwapchainKHR swapchain_{};
    VkFormat swapchain_image_format_{};
    std::vector<VkImage> swapchain_images_{};
    std::vector<VkImageView> swapchain_image_views_{};


public:
    Application();
    ~Application();

    void run();

private:
    void init_vk_device();
    void init_swapchain();
    void init_command();
};

#endif // APPLICATION_HPP