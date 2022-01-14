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

    init_vk_device();
    init_swapchain();
    init_command();
    init_renderpass();
    init_framebuffer();
    init_sync_structures();
}

Application::~Application() {

    vkDestroyFence(device_, frame_data_.render_fence_, nullptr);
    vkDestroySemaphore(device_, frame_data_.render_semaphore_, nullptr);
    vkDestroySemaphore(device_, frame_data_.present_semaphore_, nullptr);

    vkDestroyRenderPass(device_, render_pass_, nullptr);

    vkDestroyCommandPool(device_, command_pool_, nullptr);

    for(auto &framebuffer : framebuffers_) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }

    for(auto &image_view : swapchain_image_views_) {
        vkDestroyImageView(device_, image_view, nullptr);
    }
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);

    vkDestroyDevice(device_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkb::destroy_debug_utils_messenger(instance_, debug_messenger_, nullptr);
    vkDestroyInstance(instance_, nullptr);

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Application::run() {
    while (!glfwWindowShouldClose(window_)) {
        render();
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

    const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = command_pool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(device_, &command_buffer_allocate_info, &command_buffer_);
}

void Application::init_renderpass() {
    const VkAttachmentDescription color_attachment = {
        .format = swapchain_image_format_,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
    };

    const VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_);
}

void Application::init_framebuffer() {
    VkFramebufferCreateInfo framebuffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = render_pass_,
        .attachmentCount = 1,
        .width = window_extent_.width, 
        .height = window_extent_.height, 
        .layers = 1,
    };
    
    const auto swapchain_imagecount = static_cast<std::uint32_t>(swapchain_images_.size());
    framebuffers_ = std::vector<VkFramebuffer>(swapchain_imagecount);

    for(uint32_t i = 0; i < swapchain_imagecount; ++i) {
        framebuffer_create_info.pAttachments = &swapchain_image_views_[i];
        vkCreateFramebuffer(device_, &framebuffer_create_info, nullptr, &framebuffers_[i]);
    }
}

void Application::init_sync_structures() {
    const VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &frame_data_.render_semaphore_);
    vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &frame_data_.present_semaphore_);

    const VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    vkCreateFence(device_, &fence_create_info, nullptr, &frame_data_.render_fence_);
}

void Application::render() {

    vkWaitForFences(device_, 1, &frame_data_.render_fence_, true, 1000000000);
    vkResetFences(device_, 1, &frame_data_.render_fence_);

    uint32_t swapchain_image_index = 0;
    vkAcquireNextImageKHR(device_, swapchain_, 1000000000, frame_data_.present_semaphore_, nullptr, &swapchain_image_index);
    
    vkResetCommandBuffer(command_buffer_, 0);

    const VkCommandBuffer cmd = command_buffer_;

    const VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(cmd, &cmd_begin_info);

    const float flash = std::abs(std::sin(static_cast<float>(frame_number_) / 120.f));
    const VkClearValue clear_value = {.color = {{0.0f, 0.0f, flash, 1.0f}}};

    const VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass_,
        .renderArea.offset.x = 0,
        .renderArea.offset.y = 0,
        .renderArea.extent = window_extent_,
        .framebuffer = framebuffers_[swapchain_image_index],
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };

    vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame_data_.present_semaphore_, 
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer_,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &frame_data_.render_semaphore_,
    };

    vkQueueSubmit(graphics_queue_, 1, &submit_info, frame_data_.render_fence_);

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .pSwapchains = &swapchain_,
        .swapchainCount = 1,
        .pWaitSemaphores = &frame_data_.render_semaphore_,
        .waitSemaphoreCount = 1,
        .pImageIndices = &swapchain_image_index,
    };

    vkQueuePresentKHR(present_queue_, &present_info);

    ++frame_number_;
}

void Application::init_graphics_pipeline() {
    
}
