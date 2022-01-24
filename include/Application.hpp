#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"

#include "Camera.hpp"
#include "Terrain.hpp"
#include "Window.hpp"
#include "Core.hpp"
#include "Buffer.hpp"
#include "Swapchain.hpp"
#include "ShaderModule.hpp"
#include "GraphicsPipeline.hpp"
#include "Texture.hpp"

#include <fmt/core.h>
#include <glm/glm.hpp>

#include <vector>
#include <cmath>

#define VK_CHECK(x)                                     \
    do {                                                \
        VkResult err = x;                               \
        if(err) { fmt::print("Vulkan error {}", err); } \
    } while(0)                                          \


constexpr std::uint32_t frames_in_flight = 2;

struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

/*struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};*/

struct FrameData {
    VkSemaphore render_semaphore{};
    VkSemaphore present_semaphore{};
    VkFence render_fence{};

    VkCommandPool command_pool{};
    VkCommandBuffer command_buffer{};

    vkc::AllocatedBuffer camera_buffer{};
	VkDescriptorSet global_descriptor{};
};

class Application {

    Window* window_;
    VkExtent2D window_extent_{}; 

    vkc::Core* vk_core_;
    vkc::Swapchain* vk_swapchain_;

    VkRenderPass render_pass_{};
    std::vector<VkFramebuffer> framebuffers_{};

    FrameData frame_data_[frames_in_flight]{};
    uint32_t frame_number_ = 0;

    VkPipelineLayout graphics_pipeline_layout_{};
    VkPipeline graphics_pipeline_{};

    VkImageView depth_image_view_{};
    vkc::AllocatedImage depth_image_{};
    VkFormat depth_image_format_{};

    VkDescriptorSetLayout global_descriptor_set_layout_;
    VkDescriptorPool descriptor_pool_;

    Terrain terrain_{};

    Texture texture_;
    VkSampler textureSampler = VK_NULL_HANDLE;

    Camera cam_;

    bool mouse_dragging_{false};
    float last_mouse_pos_x_{};
    float last_mouse_pos_y_{};

public:
    Application();
    ~Application();

    void run();

    void update_camera_position(int key);

    void process_mouse_motion(float x, float y);

    void mouse_dragging(bool is_dragging) { 
        mouse_dragging_ = is_dragging; 
        last_mouse_pos_y_ = static_cast<float>(window_extent_.height / 2); 
        last_mouse_pos_x_ = static_cast<float>(window_extent_.width / 2);
    }
    bool is_mouse_dragging() const { return mouse_dragging_; }

private:
    void init_depthbuffer();
    void init_command();
    void init_renderpass();
    void init_framebuffer();
    void init_sync_structures();
    void init_texture_image();
    void init_descriptors();
    void init_graphics_pipeline();

    FrameData& get_current_frame();
    void transitionImageLayout(vkc::AllocatedImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(vkc::AllocatedBuffer buffer, vkc::AllocatedImage image, uint32_t width, uint32_t height);

    void render();
    void load_mesh();
    void upload_mesh(Mesh& mesh);
};

#endif // APPLICATION_HPP