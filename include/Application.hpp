#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#include <fmt/core.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

#define VK_CHECK(x)                                     \
    do {                                                \
        VkResult err = x;                               \
        if(err) { fmt::print("Vulkan error {}", err); } \
    } while(0)                                          \


constexpr std::uint32_t frames_in_flight = 2;

struct GPUCameraData{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};

struct FrameData {
    VkSemaphore render_semaphore{};
    VkSemaphore present_semaphore{};
    VkFence render_fence{};

    VkCommandPool command_pool{};
    VkCommandBuffer command_buffer{};

    AllocatedBuffer camera_buffer{};
	VkDescriptorSet global_descriptor{};
};

struct BufferCreateInfo {
    size_t alloc_size = 0;
    VkBufferUsageFlags usage = 0;
    VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_UNKNOWN;
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
    std::vector<std::uint32_t> indices_;
    AllocatedBuffer vertex_buffer_;
    AllocatedBuffer index_buffer_;
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

    VkRenderPass render_pass_{};
    std::vector<VkFramebuffer> framebuffers_{};

    VkSwapchainKHR swapchain_{};
    VkFormat swapchain_image_format_{};
    std::vector<VkImage> swapchain_images_{};
    std::vector<VkImageView> swapchain_image_views_{};
    FrameData frame_data_[frames_in_flight]{};
    uint32_t frame_number_ = 0;

    VkPipelineLayout graphics_pipeline_layout_{};
    VkPipeline graphics_pipeline_{};

    VkImageView depth_image_view_{};
    AllocatedImage depth_image_{};
    VkFormat depth_image_format_{};

    VkDescriptorSetLayout global_descriptor_set_layout_;
    VkDescriptorPool descriptor_pool_;

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
    void init_descriptors();
    void init_graphics_pipeline();

    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    FrameData& get_current_frame();
    AllocatedBuffer create_buffer(const BufferCreateInfo& buffer_create_info);
    AllocatedBuffer create_buffer_from_data(const BufferCreateInfo& buffer_create_info, void* data);

    void render();
    void load_mesh();
    void upload_mesh(Mesh& mesh);
};

#endif // APPLICATION_HPP