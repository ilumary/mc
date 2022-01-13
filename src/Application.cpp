#include "../include/Application.hpp"

namespace {
    auto create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr) -> VkSurfaceKHR {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
        if (err) {
            const char* error_msg;
            int ret = glfwGetError(&error_msg);
            if (ret != 0) {
                fmt::print("{} ", ret);
                if (error_msg != nullptr) { fmt::print("{} ", error_msg); }
                fmt::print("\n");
            }
            surface = VK_NULL_HANDLE;
        }
        return surface;
    }
}

Application::Application() {
    if (!glfwInit()) { std::exit(1); }
    fmt::print("GLFW initialised\n");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window_extent_ = VkExtent2D{800, 600};
    
    window_ = glfwCreateWindow(window_extent_.width, window_extent_.height, "Voxel Simulation", nullptr, nullptr);
    if(!window_) { std::exit(1); }
    
    glfwMakeContextCurrent(window_);

    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t i = 0; i < count; ++i) {
        std::cout << extensions[i] << std::endl;
    }

    init_vk_device();
    init_swapchain();
    init_command();
}

Application::~Application() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Application::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

void Application::init_vk_device() {
    auto instance_ret = vkb::InstanceBuilder{}.use_default_debug_messenger().request_validation_layers().build();
    if (!instance_ret) {
        fmt::print("{}\n", instance_ret.error().message());
        std::exit(1);
    }
        
    instance_ = instance_ret->instance;
    surface_ = create_surface_glfw(instance_, window_);
    debug_messenger_ = instance_ret->debug_messenger;

    vkb::PhysicalDeviceSelector phys_device_selector(instance_ret.value());
    auto phys_device_ret = phys_device_selector.set_surface(surface_).select();
    if (!phys_device_ret) {
        fmt::print("{}\n", instance_ret.error().message());
        std::exit(1);
    }
    vkb::PhysicalDevice vkb_physical_device = phys_device_ret.value();
    physical_device_ = vkb_physical_device.physical_device;

    vkb::DeviceBuilder device_builder{ vkb_physical_device };
    auto device_ret = device_builder.build();
    if (!device_ret) {
        fmt::print("{}\n", device_ret.error().message());
        std::exit(1);
    }
    auto vkb_device = device_ret.value();
    device_ = vkb_device.device;
    
    graphics_queue_ = vkb_device.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family_index_ = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    present_queue_ = vkb_device.get_queue(vkb::QueueType::present).value();
}

void Application::init_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{physical_device_, device_, surface_};
    
    vkb::Swapchain vkb_swapchain = swapchain_builder
                                    .use_default_format_selection()
                                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                    .set_desired_extent(window_extent_.width, window_extent_.height)
                                    .build()
                                    .value();
    
    swapchain_ = vkb_swapchain.swapchain;
    swapchain_images_ = vkb_swapchain.get_images().value();
    swapchain_image_views_ = vkb_swapchain.get_image_views().value();
    swapchain_image_format_ = vkb_swapchain.image_format;
}

void Application::init_command() {
    const VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family_index_,
    };
    
    vkCreateCommandPool(device_, &command_pool_create_info, nullptr, &command_pool_);
}
