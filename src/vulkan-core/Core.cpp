#include "Core.hpp"

#include <VkBootstrap.h>
#include <fmt/format.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vkc {

    Core::Core(Window& window) {
        auto instance_ret = vkb::InstanceBuilder{}.use_default_debug_messenger().request_validation_layers().build();
        if (!instance_ret) {
            fmt::print("{}\n", instance_ret.error().message());
            std::exit(1);
        }
            
        instance_ = instance_ret->instance;
        glfwCreateWindowSurface(instance_, window.glfw_window(), nullptr, &surface_);
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

        const VmaAllocatorCreateInfo allocator_create_info = {
            .instance = instance_,
            .physicalDevice = physical_device_,
            .device = device_,
        };

        vmaCreateAllocator(&allocator_create_info, &allocator_);
    }

    Core::~Core(){
        if (!instance_) return;

        vmaDestroyAllocator(allocator_);

        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkb::destroy_debug_utils_messenger(instance_, debug_messenger_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }
}