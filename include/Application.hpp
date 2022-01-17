#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#include <fmt/core.h>
#include <glm/vec3.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

#define VK_CHECK(x)                                     \
    do {                                                \
        VkResult err = x;                               \
        if(err) { fmt::print("Vulkan error {}", err); } \
    } while(0)                                          \

struct FrameData {
    VkSemaphore render_semaphore_{};
    VkSemaphore present_semaphore_{};
    VkFence render_fence_{};
};

struct AllocatedBuffer {
    VkBuffer buffer_;
    VmaAllocation allocation_;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VkVertexInputBindingDescription binding_description();
    static std::vector<VkVertexInputAttributeDescription> attributes_description();
};

struct Mesh {
    std::vector<Vertex> vertices_;
    AllocatedBuffer vertex_buffer_;
};

class Application {

    GLFWwindow* window_ = nullptr;
    VkExtent2D window_extent_{};

    VkDebugUtilsMessengerEXT debug_messenger_{};

    VkInstance instance_{};
    VkPhysicalDevice physical_device_{};
    VkSurfaceKHR surface_{};
    VkDevice device_{};
    VkQueue graphics_queue_{};
    uint32_t graphics_queue_family_index_ = 0;
    VkQueue present_queue_{};

    VmaAllocator allocator_{};

    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;

    VkRenderPass render_pass_{};
    std::vector<VkFramebuffer> framebuffers_{};

    VkSwapchainKHR swapchain_{};
    VkFormat swapchain_image_format_{};
    std::vector<VkImage> swapchain_images_{};
    std::vector<VkImageView> swapchain_image_views_{};
    FrameData frame_data_{};
    uint32_t frame_number_ = 0;

    VkPipelineLayout graphics_pipeline_layout_{};
    VkPipeline graphics_pipeline_{};

    Mesh terrain_mesh_{};

public:
    Application();
    ~Application();

    void run();

private:
    void init_vk_device();
    void init_swapchain();
    void init_command();
    void init_renderpass();
    void init_framebuffer();
    void init_sync_structures();
    void init_graphics_pipeline();

    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    void render();
    void load_mesh();
    void upload_mesh(Mesh& mesh);
};

#endif // APPLICATION_HPP