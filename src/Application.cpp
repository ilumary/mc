#include "../include/Application.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
        cam_.update(delta_time_);
        cam_.keys.up = false;
    } else if( key == GLFW_KEY_A) {
        cam_.keys.left = true;
        cam_.update(delta_time_);
        cam_.keys.left = false;
    } else if( key == GLFW_KEY_S) {
        cam_.keys.down = true;
        cam_.update(delta_time_);
        cam_.keys.down = false;
    } else if( key == GLFW_KEY_D) {
        cam_.keys.right = true;
        cam_.update(delta_time_);
        cam_.keys.right = false;
    }
}

Application::Application() {
    window_extent_ = VkExtent2D{1400, 900};
    window_ = new Window(window_extent_.width, window_extent_.height, "");
    vk_core_ = new vkc::Core(*window_);
    vk_swapchain_ = new vkc::Swapchain(*vk_core_, window_extent_);

    cam_.setMovementSpeed(100.f);
    cam_.setPosition({ 0.f, 2.f, 0.f });
    cam_.setRotation({ 0.f, 90.f, 0.f });
    cam_.setPerspective(70.f, 1400.f / 900.f, 0.1f, 200.f);
    cam_.type = Camera::CameraType::firstperson;
    cam_.update(1.f);

    glfwMakeContextCurrent(window_->glfw_window());
    glfwSetWindowUserPointer(window_->glfw_window(), this);
    glfwSetKeyCallback(window_->glfw_window(), key_callback);
    glfwSetCursorPosCallback(window_->glfw_window(), cursor_position_callback);
    glfwSetMouseButtonCallback(window_->glfw_window(), mouse_button_callback);
    
    init_depthbuffer();
    vk_renderpass_ = new vkc::Renderpass(*vk_core_, vk_swapchain_->format(), depth_image_format_);
    init_command();
    init_framebuffer();
    init_sync_structures();
    init_texture_image();
    texture_.createSampler(*vk_core_, textureSampler);
    init_descriptors();
    init_graphics_pipeline();
    load_mesh();
}

Application::~Application() {
    world_mesh_->destroy(*vk_core_);

    vkDestroyPipeline(vk_core_->device(), graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(vk_core_->device(), graphics_pipeline_layout_, nullptr);

    vkDestroyRenderPass(vk_core_->device(), vk_renderpass_->renderpass(), nullptr);

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
        update();
        render();
        window_->swap_buffers();
        window_->pull_events();
    }
}

void Application::update() {
    double actual = glfwGetTime();
    double delta = actual - last_cout_;
	delta_time_ = (actual - time_) / 1000.0f;
    frame_number_per_second_ += 1;

    if(delta >= 1.f) { 
        double fps = double(frame_number_per_second_) / delta;

        std::stringstream ss;
        ss << "Voxel Simulation " << " [" << fps << " FPS]";

        glfwSetWindowTitle(window_->glfw_window(), ss.str().c_str());

        frame_number_per_second_ = 0;
        last_cout_ = actual;
    }
    
    if(delta_time_ < LOW_LIMIT) {
        delta_time_ = LOW_LIMIT;
    } else if(delta_time_ > HIGH_LIMIT) {
        delta_time_ = HIGH_LIMIT;
    }
	time_ = actual;
}

void Application::init_depthbuffer() {
    const VkExtent3D depth_extent = {window_extent_.width, window_extent_.height, 1};

    depth_image_format_ = VK_FORMAT_D32_SFLOAT;

    vkc::create_image(*vk_core_, {
        .extent = depth_extent,
        .format = depth_image_format_,
        .image_tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .required_flags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    }, depth_image_);

    vkc::create_image_view(*vk_core_, {
        .image = depth_image_.image,
        .format = depth_image_format_,
        .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
    }, depth_image_view_);
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

void Application::init_framebuffer() {
    VkFramebufferCreateInfo framebuffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = vk_renderpass_->renderpass(),
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

void Application::init_texture_image() {
    int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load("../../textures/minecraft.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels) { fmt::print("Failed to load texture file\n"); }

    void* pixel_ptr = pixels;
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

	vkc::AllocatedBuffer stagingBuffer = vkc::create_buffer_from_data(*vk_core_, {
        .alloc_size = imageSize, 
        .memory_usage = VMA_MEMORY_USAGE_CPU_ONLY,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    }, pixel_ptr);

    vkc::create_image(*vk_core_, {
        .extent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1},
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .image_tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage_flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    }, texture_.image_);

    for (std::uint32_t i = 0; i < frames_in_flight; ++i) {

        vkc::transitionImageLayout(*vk_core_, frame_data_[i].command_pool, texture_.image_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vkc::copyBufferToImage(*vk_core_, frame_data_[i].command_pool, stagingBuffer, texture_.image_, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        vkc::transitionImageLayout(*vk_core_, frame_data_[i].command_pool, texture_.image_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    }

    vkc::destroy_buffer(*vk_core_, stagingBuffer);

    vkc::create_image_view(*vk_core_, {
        .image = texture_.image_.image, 
        .format = VK_FORMAT_R8G8B8A8_UNORM, 
        .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    }, texture_.image_view_);
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

    VkDescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImmutableSamplers = nullptr,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding bindings[2] = {camera_buffer_binding, samplerLayoutBinding};

	const VkDescriptorSetLayoutCreateInfo set_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = 2,
        .flags = 0,
        .pBindings = &bindings[0],
    };

	std::vector<VkDescriptorPoolSize> sizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(vk_swapchain_->images().size()) },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(vk_swapchain_->images().size()) }
    };

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = static_cast<uint32_t>(vk_swapchain_->images().size()),
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

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture_.image_view_;
        imageInfo.sampler = textureSampler;

        VkWriteDescriptorSet write_set_uniform_buffer = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstBinding = 0,
            .dstSet = frame_data_[i].global_descriptor,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &buffer_info,
        };

        VkWriteDescriptorSet write_set_uv_texture = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .dstSet = frame_data_[i].global_descriptor,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };

        VkWriteDescriptorSet write_sets[2] = { write_set_uniform_buffer, write_set_uv_texture };

        vkUpdateDescriptorSets(vk_core_->device(), 2, &write_sets[0], 0, nullptr);
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
        .render_pass = vk_renderpass_->renderpass(),
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
        .renderPass = vk_renderpass_->renderpass(),
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
    
    vkCmdBindVertexBuffers(cmd, 0, 1, &world_mesh_->vertex_buffer.buffer, &offset);
    
    vkCmdBindIndexBuffer(cmd, world_mesh_->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout_, 0, 1, &current_frame_data.global_descriptor, 0, nullptr);

    vkCmdDrawIndexed(cmd, static_cast<std::uint32_t>(world_mesh_->indices.size()), 1, 0, 0, 0);

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
    world_mesh_ = world_.getWorldMesh(cam_.position, 1);

    upload_mesh(world_mesh_);
}

void Application::upload_mesh(Mesh* mesh) {
    mesh->vertex_buffer = vkc::create_buffer_from_data(*vk_core_, {
        .alloc_size = mesh->vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    }, mesh->vertices.data());

    mesh->index_buffer = vkc::create_buffer_from_data(*vk_core_, {
        .alloc_size = mesh->indices.size() * sizeof(std::uint32_t),
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    }, mesh->indices.data());
}

FrameData& Application::get_current_frame() {
    return frame_data_[frame_number_ % frames_in_flight];
}