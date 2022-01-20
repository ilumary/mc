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
    if (!glfwInit()) { std::exit(1); }
    fmt::print("GLFW initialised\n");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window_extent_ = VkExtent2D{800, 600};
    
    window_ = glfwCreateWindow(window_extent_.width, window_extent_.height, "Voxel Simulation", nullptr, nullptr);
    if(!window_) { std::exit(1); }

    cam_.setPosition({ 0.f, 0.f,-2.f });
    cam_.setPerspective(70.f, 800.f / 600.f, 0.1f, 200.f);
    cam_.type = Camera::CameraType::firstperson;

    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, key_callback);
    glfwSetCursorPosCallback(window_, cursor_position_callback);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);

    init_vk_device();
    init_swapchain();
    init_command();
    init_renderpass();
    init_framebuffer();
    init_sync_structures();
    init_descriptors();
    init_graphics_pipeline();
    load_mesh();
}

Application::~Application() {
    //vmaDestroyBuffer(allocator_, terrain_mesh_.index_buffer_.buffer, terrain_mesh_.index_buffer_.allocation);
    vmaDestroyBuffer(allocator_, terrain_mesh_.vertex_buffer_.buffer, terrain_mesh_.vertex_buffer_.allocation);

    vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(device_, graphics_pipeline_layout_, nullptr);

    vkDestroyRenderPass(device_, render_pass_, nullptr);

    for(auto& frame_data : frame_data_) {
        vmaDestroyBuffer(allocator_, frame_data.camera_buffer.buffer, frame_data.camera_buffer.allocation);
        vkDestroyFence(device_, frame_data.render_fence, nullptr);
        vkDestroySemaphore(device_, frame_data.render_semaphore, nullptr);
        vkDestroySemaphore(device_, frame_data.present_semaphore, nullptr);
        vkDestroyCommandPool(device_, frame_data.command_pool, nullptr);
    }

    vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(device_, global_descriptor_set_layout_, nullptr);

    for(auto &framebuffer : framebuffers_) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }

    for(auto &image_view : swapchain_image_views_) {
        vkDestroyImageView(device_, image_view, nullptr);
    }
    vkDestroyImageView(device_, depth_image_view_, nullptr);
    vmaDestroyImage(allocator_, depth_image_.image, depth_image_.allocation);
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);

    vmaDestroyAllocator(allocator_);

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
    glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
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

    VK_CHECK(vmaCreateAllocator(&allocator_create_info, &allocator_));
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

    vmaCreateImage(allocator_, &depth_image_create_info, &dimg_allocinfo, &depth_image_.image, &depth_image_.allocation, nullptr);

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

    vkCreateImageView(device_, &depth_view_create_info, nullptr, &depth_image_view_);
}

void Application::init_command() {
    const VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family_index_,
    };
    
    for(std::uint32_t i = 0; i < frames_in_flight; ++i) {
        auto& frame_data = frame_data_[i];
        vkCreateCommandPool(device_, &command_pool_create_info, nullptr, &frame_data.command_pool);

        const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = frame_data.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(device_, &command_buffer_allocate_info, &frame_data.command_buffer);
    }
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
        const VkImageView attachments[] = {swapchain_image_views_[i], depth_image_view_};
        framebuffer_create_info.pAttachments = &attachments[0];
        framebuffer_create_info.attachmentCount = 2;
        vkCreateFramebuffer(device_, &framebuffer_create_info, nullptr, &framebuffers_[i]);
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
        vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &frame_data.render_semaphore);
        vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &frame_data.present_semaphore);
        vkCreateFence(device_, &fence_create_info, nullptr, &frame_data.render_fence);
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

	VK_CHECK(vkCreateDescriptorPool(device_, &descriptor_pool_create_info, nullptr, &descriptor_pool_));

	VK_CHECK(vkCreateDescriptorSetLayout(device_, &set_layout_info, nullptr, &global_descriptor_set_layout_));

    for (std::uint32_t i = 0; i < frames_in_flight; ++i) {
        frame_data_[i].camera_buffer = create_buffer({
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

        vkAllocateDescriptorSets(device_, &alloc_info, &frame_data_[i].global_descriptor);

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

        vkUpdateDescriptorSets(device_, 1, &write_set, 0, nullptr);
    }
}

std::vector<char> Application::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error ("failed to open file!");
	}

	size_t file_size = (size_t)file.tellg ();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(file_size));

	file.close();

	return buffer;
}

VkShaderModule Application::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size ();
	create_info.pCode = reinterpret_cast<const uint32_t*> (code.data ());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device_, &create_info, nullptr, &shaderModule) != VK_SUCCESS) {
		return VK_NULL_HANDLE; 
	}

	return shaderModule;
}

void Application::init_graphics_pipeline() {
    auto vert_shader_code = readFile("../../shaders/bin/vert.spv");
    auto frag_shader_code = readFile("../../shaders/bin/frag.spv");

    VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
    VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    static const auto vertex_binding_description = Vertex::binding_description();
    static const auto vertex_attributes_description = Vertex::attributes_description();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes_description.size()),
        .pVertexAttributeDescriptions = vertex_attributes_description.data(),
    };
    
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) window_extent_.width,
        .height = (float) window_extent_.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = window_extent_,
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };
    

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &global_descriptor_set_layout_,
        .pushConstantRangeCount = 0,
    };

    VK_CHECK(vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &graphics_pipeline_layout_));

    const VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f, 
        .maxDepthBounds = 1.0f,
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil_info,
        .pColorBlendState = &color_blending,
        .layout = graphics_pipeline_layout_,
        .renderPass = render_pass_,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    VK_CHECK(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline_));

    vkDestroyShaderModule(device_, frag_shader_module, nullptr);
    vkDestroyShaderModule(device_, vert_shader_module, nullptr);
}

void Application::render() {
    auto current_frame_data = get_current_frame();

	GPUCameraData cam_data = {
        .proj = cam_.matrices.perspective,
        .view = cam_.matrices.view,
        .viewproj = cam_.matrices.perspective * cam_.matrices.view,
    };

	void* data;
	vmaMapMemory(allocator_, current_frame_data.camera_buffer.allocation, &data);
	memcpy(data, &cam_data, sizeof(GPUCameraData));
	vmaUnmapMemory(allocator_, current_frame_data.camera_buffer.allocation);

    vkWaitForFences(device_, 1, &current_frame_data.render_fence, true, 1000000000);
    vkResetFences(device_, 1, &current_frame_data.render_fence);

    uint32_t swapchain_image_index = 0;
    vkAcquireNextImageKHR(device_, swapchain_, 1000000000, current_frame_data.present_semaphore, nullptr, &swapchain_image_index);
    
    vkResetCommandBuffer(current_frame_data.command_buffer, 0);

    const VkCommandBuffer cmd = current_frame_data.command_buffer;

    const VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(cmd, &cmd_begin_info);

    const float flash = std::abs(std::sin(static_cast<float>(frame_number_) / 120.f));
    const VkClearValue clear_color_value = {.color = {{0.0f, 0.0f, flash, 1.0f}}};
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
    vkCmdBindVertexBuffers(cmd, 0, 1, &terrain_mesh_.vertex_buffer_.buffer, &offset);

    //vkCmdBindIndexBuffer(cmd, terrain_mesh_.index_buffer_.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout_, 0, 1, &current_frame_data.global_descriptor, 0, nullptr);

    //vkCmdDrawIndexed(cmd, static_cast<std::uint32_t>(terrain_mesh_.indices_.size()), 1, 0, 0, 0);

    vkCmdDraw(cmd, static_cast<std::uint32_t>(terrain_mesh_.vertices_.size()), 1, 0, 0);

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

    vkQueueSubmit(graphics_queue_, 1, &submit_info, current_frame_data.render_fence);

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .pSwapchains = &swapchain_,
        .swapchainCount = 1,
        .pWaitSemaphores = &current_frame_data.render_semaphore,
        .waitSemaphoreCount = 1,
        .pImageIndices = &swapchain_image_index,
    };

    vkQueuePresentKHR(present_queue_, &present_info);

    ++frame_number_;
}

void Application::load_mesh() {

    //std::vector<Vertex> vertices = generate_terrain();

    terrain_mesh_.vertices_ = generate_terrain();

    /*terrain_mesh_.vertices_.resize(3);

    terrain_mesh_.vertices_[0].position = {0.7f, 0.7f, 0.0f};
    terrain_mesh_.vertices_[1].position = {-0.7f, 0.7f, 0.0f};
    terrain_mesh_.vertices_[2].position = {0.f, -0.7f, 0.0f};

    terrain_mesh_.vertices_[0].color = {1.f, 0.f, 0.0f};
    terrain_mesh_.vertices_[1].color = {0.f, 1.f, 0.0f};
    terrain_mesh_.vertices_[2].color = {0.f, 0.f, 1.0f};*/

    //terrain_mesh_.indices_ = {0, 1, 2};

    upload_mesh(terrain_mesh_);
}

void Application::upload_mesh(Mesh& mesh) {
    
    mesh.vertex_buffer_ = create_buffer_from_data({
        .alloc_size = mesh.vertices_.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    }, mesh.vertices_.data());

    /*mesh.index_buffer_ = create_buffer_from_data({
        .alloc_size = mesh.indices_.size() * sizeof(std::uint32_t),
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    }, mesh.indices_.data());*/
}

FrameData& Application::get_current_frame() {
    return frame_data_[frame_number_ % frames_in_flight];
}

AllocatedBuffer Application::create_buffer(const BufferCreateInfo& buffer_create_info) {
	VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = buffer_create_info.alloc_size,
        .usage = buffer_create_info.usage,
    };

	VmaAllocationCreateInfo vmaalloc_info = {.usage = buffer_create_info.memory_usage,};

	AllocatedBuffer new_buffer;

	VK_CHECK(vmaCreateBuffer(allocator_, &buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation, nullptr));

	return new_buffer;
}

AllocatedBuffer Application::create_buffer_from_data(const BufferCreateInfo& buffer_create_info, void* data) {
    AllocatedBuffer buffer = create_buffer(buffer_create_info);
    void* mapped_mem = nullptr;
	vmaMapMemory(allocator_, buffer.allocation, &mapped_mem);
	memcpy(mapped_mem, data, buffer_create_info.alloc_size);
	vmaUnmapMemory(allocator_, buffer.allocation);
    return buffer; 
}