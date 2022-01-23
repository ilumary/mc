#include "../include/Application.hpp"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        app->update_camera_position(key);
    } 
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if(app->is_mouse_dragging()) {
        app->process_mouse_motion(static_cast<float>(xpos), static_cast<float>(ypos));
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if(button == GLFW_MOUSE_BUTTON_RIGHT) {
        if(action == GLFW_PRESS) {
            app->mouse_dragging(true);
        } else if (action == GLFW_RELEASE) {
            app->mouse_dragging(false);
        }
    }
}

void Application::process_mouse_motion(float x, float y) {
    cam_.rotate(glm::vec3{(y - last_mouse_pos_y_) * 0.1f, (x - last_mouse_pos_x_) * 0.1f, 0.0f});
    last_mouse_pos_x_ = x;
    last_mouse_pos_y_ = y;
}

void Application::update_camera_position(int key) {
    if( key == GLFW_KEY_W) {
        cam_.keys.up = true;
        cam_.update(0.5f);
        cam_.keys.up = false;
    } else if( key == GLFW_KEY_A) {
        cam_.keys.left = true;
        cam_.update(0.5f);
        cam_.keys.left = false;
    } else if( key == GLFW_KEY_S) {
        cam_.keys.down = true;
        cam_.update(0.5f);
        cam_.keys.down = false;
    } else if( key == GLFW_KEY_D) {
        cam_.keys.right = true;
        cam_.update(0.5f);
        cam_.keys.right = false;
    }
}

Application::Application() {
    window_extent_ = VkExtent2D{1400, 900};
    window_ = new Window(window_extent_.width, window_extent_.height, "Voxel Simulation");
    vk_core_ = new vkc::Core(*window_);
    vk_swapchain_ = new vkc::Swapchain(*vk_core_, window_extent_);

    cam_.setPosition({ 0.f, 0.f,-2.f });
    cam_.setPerspective(70.f, 1400.f / 900.f, 0.1f, 200.f);
    cam_.type = Camera::CameraType::firstperson;

    glfwMakeContextCurrent(window_->glfw_window());
    glfwSetWindowUserPointer(window_->glfw_window(), this);
    glfwSetKeyCallback(window_->glfw_window(), key_callback);
    glfwSetCursorPosCallback(window_->glfw_window(), cursor_position_callback);
    glfwSetMouseButtonCallback(window_->glfw_window(), mouse_button_callback);
    
    init_depthbuffer();
    init_command();
    init_renderpass();
    init_framebuffer();
    init_sync_structures();
    init_descriptors();
    init_graphics_pipeline();
    load_mesh();
}

Application::~Application() {
    terrain_.destroy(*vk_core_);

    vkDestroyPipeline(vk_core_->device(), graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(vk_core_->device(), graphics_pipeline_layout_, nullptr);

    vkDestroyRenderPass(vk_core_->device(), render_pass_, nullptr);

    for(auto& frame_data : frame_data_) {
        vmaDestroyBuffer(vk_core_->allocator(), frame_data.camera_buffer.buffer, frame_data.camera_buffer.allocation);
        vkDestroyFence(vk_core_->device(), frame_data.render_fence, nullptr);
        vkDestroySemaphore(vk_core_->device(), frame_data.render_semaphore, nullptr);
        vkDestroySemaphore(vk_core_->device(), frame_data.present_semaphore, nullptr);
        vkDestroyCommandPool(vk_core_->device(), frame_data.command_pool, nullptr);
    }

    vkDestroyDescriptorPool(vk_core_->device(), descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(vk_core_->device(), global_descriptor_set_layout_, nullptr);

    for(auto &framebuffer : framebuffers_) {
        vkDestroyFramebuffer(vk_core_->device(), framebuffer, nullptr);
    }

    vkDestroyImageView(vk_core_->device(), depth_image_view_, nullptr);
    vmaDestroyImage(vk_core_->allocator(), depth_image_.image, depth_image_.allocation);
}

void Application::run() {
    while (!window_->should_close()) {
        render();
        window_->swap_buffers();
        window_->pull_events();
    }
}

void Application::init_depthbuffer() {
    depth_image_format_ = VK_FORMAT_D32_SFLOAT;

    const VkExtent3D depth_extent = {window_extent_.width, window_extent_.height, 1};

    const VkImageCreateInfo depth_image_create_info = { 
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_image_format_,
        .extent = depth_extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    VmaAllocationCreateInfo dimg_allocinfo = { 
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    vmaCreateImage(vk_core_->allocator(), &depth_image_create_info, &dimg_allocinfo, &depth_image_.image, &depth_image_.allocation, nullptr);

    const VkImageViewCreateInfo depth_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = depth_image_.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depth_image_format_,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vkCreateImageView(vk_core_->device(), &depth_view_create_info, nullptr, &depth_image_view_);
}

void Application::init_command() {
    const VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vk_core_->graphics_queue_family_index(),
    };
    
    for(std::uint32_t i = 0; i < frames_in_flight; ++i) {
        auto& frame_data = frame_data_[i];
        vkCreateCommandPool(vk_core_->device(), &command_pool_create_info, nullptr, &frame_data.command_pool);

        const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = frame_data.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(vk_core_->device(), &command_buffer_allocate_info, &frame_data.command_buffer);
    }
}

void Application::init_renderpass() {
    const VkAttachmentDescription color_attachment = {
        .format = vk_swapchain_->format(),
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

    const VkAttachmentDescription depth_attachment = {
        .flags = 0,
        .format = depth_image_format_,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentReference depth_attachment_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref,
    };

    const VkAttachmentDescription attachments[] = {
        color_attachment, depth_attachment
    };

    const VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 2,
        .pAttachments = &attachments[0],
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    vkCreateRenderPass(vk_core_->device(), &render_pass_info, nullptr, &render_pass_);
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
    
    const auto swapchain_imagecount = static_cast<std::uint32_t>(vk_swapchain_->images().size());
    framebuffers_ = std::vector<VkFramebuffer>(swapchain_imagecount);

    for(uint32_t i = 0; i < swapchain_imagecount; ++i) {
        const VkImageView attachments[] = {vk_swapchain_->image_views()[i], depth_image_view_};
        framebuffer_create_info.pAttachments = &attachments[0];
        framebuffer_create_info.attachmentCount = 2;
        vkCreateFramebuffer(vk_core_->device(), &framebuffer_create_info, nullptr, &framebuffers_[i]);
    }
}

void Application::init_sync_structures() {
    const VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    const VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    for(auto& frame_data : frame_data_) {
        vkCreateSemaphore(vk_core_->device(), &semaphore_create_info, nullptr, &frame_data.render_semaphore);
        vkCreateSemaphore(vk_core_->device(), &semaphore_create_info, nullptr, &frame_data.present_semaphore);
        vkCreateFence(vk_core_->device(), &fence_create_info, nullptr, &frame_data.render_fence);
    }
}

void Application::init_descriptors() {
	const VkDescriptorSetLayoutBinding camera_buffer_binding = {
        .binding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

	const VkDescriptorSetLayoutCreateInfo set_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = 1,
        .flags = 0,
        .pBindings = &camera_buffer_binding,
    };

	std::vector<VkDescriptorPoolSize> sizes = {{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }};

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = 10,
        .poolSizeCount = static_cast<std::uint32_t>(sizes.size()),
        .pPoolSizes = sizes.data(),
    };

	VK_CHECK(vkCreateDescriptorPool(vk_core_->device(), &descriptor_pool_create_info, nullptr, &descriptor_pool_));

	VK_CHECK(vkCreateDescriptorSetLayout(vk_core_->device(), &set_layout_info, nullptr, &global_descriptor_set_layout_));

    for (std::uint32_t i = 0; i < frames_in_flight; ++i) {
        frame_data_[i].camera_buffer = create_buffer(*vk_core_, {
            .alloc_size = sizeof(GPUCameraData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        });

        const VkDescriptorSetAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptor_pool_,
            .descriptorSetCount = 1,
            .pSetLayouts = &global_descriptor_set_layout_,
        };

        vkAllocateDescriptorSets(vk_core_->device(), &alloc_info, &frame_data_[i].global_descriptor);

        const VkDescriptorBufferInfo buffer_info = {
            .buffer = frame_data_[i].camera_buffer.buffer,
            .offset = 0,
            .range = sizeof(GPUCameraData),
        };

        VkWriteDescriptorSet write_set = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstBinding = 0,
            .dstSet = frame_data_[i].global_descriptor,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &buffer_info,
        };

        vkUpdateDescriptorSets(vk_core_->device(), 1, &write_set, 0, nullptr);
    }
}

void Application::init_graphics_pipeline() {
    vkc::ShaderModule* vert_shader_module = new vkc::ShaderModule(*vk_core_, "../../shaders/bin/vert.spv");
    vkc::ShaderModule* frag_shader_module = new vkc::ShaderModule(*vk_core_, "../../shaders/bin/frag.spv");

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module->shader_module(),
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module->shader_module(),
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    static const auto vertex_binding_description = Vertex::binding_description();
    static const auto vertex_attributes_description = Vertex::attributes_description();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &global_descriptor_set_layout_,
        .pushConstantRangeCount = 0,
    };

    VK_CHECK(vkCreatePipelineLayout(vk_core_->device(), &pipelineLayoutInfo, nullptr, &graphics_pipeline_layout_));

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes_description.size()),
        .pVertexAttributeDescriptions = vertex_attributes_description.data(),
    };

    graphics_pipeline_ = vkc::create_graphics_pipeline(*vk_core_, vkc::GraphicsPipelineCreateInfo{
        .pipeline_layout = graphics_pipeline_layout_,
        .render_pass = render_pass_,
        .window_extent = window_extent_,
        .shader_stages = &shader_stages[0],
        .shader_stage_count = 2,
        .cull_mode = vkc::CullMode::back,
    });
}

void Application::render() {
    auto current_frame_data = get_current_frame();

	GPUCameraData cam_data = {
        .proj = cam_.matrices.perspective,
        .view = cam_.matrices.view,
        .viewproj = cam_.matrices.perspective * cam_.matrices.view,
    };

	void* data;
	vmaMapMemory(vk_core_->allocator(), current_frame_data.camera_buffer.allocation, &data);
	memcpy(data, &cam_data, sizeof(GPUCameraData));
	vmaUnmapMemory(vk_core_->allocator(), current_frame_data.camera_buffer.allocation);

    vkWaitForFences(vk_core_->device(), 1, &current_frame_data.render_fence, true, 1000000000);
    vkResetFences(vk_core_->device(), 1, &current_frame_data.render_fence);

    uint32_t swapchain_image_index = 0;
    vkAcquireNextImageKHR(vk_core_->device(), vk_swapchain_->swapchain(), 1000000000, current_frame_data.present_semaphore, nullptr, &swapchain_image_index);
    
    vkResetCommandBuffer(current_frame_data.command_buffer, 0);

    const VkCommandBuffer cmd = current_frame_data.command_buffer;

    const VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(cmd, &cmd_begin_info);

    const VkClearValue clear_color_value = {.color = {{0.0f, 0.72f, 1.0f, 1.0f}}};
    const VkClearValue clear_depth_value = {.depthStencil = {.depth = 1.f}};
    const VkClearValue clear_values[] = {clear_color_value, clear_depth_value};

    const VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass_,
        .renderArea.offset.x = 0,
        .renderArea.offset.y = 0,
        .renderArea.extent = window_extent_,
        .framebuffer = framebuffers_[swapchain_image_index],
        .clearValueCount = 2,
        .pClearValues = &clear_values[0],
    };

    vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

    VkDeviceSize offset = 0;
    
    vkCmdBindVertexBuffers(cmd, 0, 1, &terrain_.vertex_buffer.buffer, &offset);
    
    vkCmdBindIndexBuffer(cmd, terrain_.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout_, 0, 1, &current_frame_data.global_descriptor, 0, nullptr);

    vkCmdDrawIndexed(cmd, static_cast<std::uint32_t>(terrain_.indices.size()), 1, 0, 0, 0);

    //vkCmdDraw(cmd, static_cast<std::uint32_t>(terrain_mesh_.vertices_.size()), 1, 0, 0);

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &current_frame_data.present_semaphore, 
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &current_frame_data.command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &current_frame_data.render_semaphore,
    };

    vkQueueSubmit(vk_core_->graphics_queue(), 1, &submit_info, current_frame_data.render_fence);

    VkSwapchainKHR helper = vk_swapchain_->swapchain();

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .pSwapchains = &helper,
        .swapchainCount = 1,
        .pWaitSemaphores = &current_frame_data.render_semaphore,
        .waitSemaphoreCount = 1,
        .pImageIndices = &swapchain_image_index,
    };

    vkQueuePresentKHR(vk_core_->present_queue(), &present_info);

    ++frame_number_;
}

void Application::load_mesh() {
    upload_mesh(terrain_);
}

void Application::upload_mesh(Mesh& mesh) {
    mesh.vertex_buffer = vkc::create_buffer_from_data(*vk_core_, {
        .alloc_size = mesh.vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    }, mesh.vertices.data());

    mesh.index_buffer = vkc::create_buffer_from_data(*vk_core_, {
        .alloc_size = mesh.indices.size() * sizeof(std::uint32_t),
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    }, mesh.indices.data());
}

FrameData& Application::get_current_frame() {
    return frame_data_[frame_number_ % frames_in_flight];
}