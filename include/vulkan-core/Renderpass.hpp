#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP

#include <vulkan/vulkan.h>

#include "Core.hpp"

namespace vkc {

class Renderpass {

    VkRenderPass render_pass{};

public:
    explicit Renderpass(vkc::Core& core, VkFormat swapchain_format, VkFormat depth_image_format);
    ~Renderpass();

    inline VkRenderPass renderpass() noexcept { return render_pass; }
};

}

#endif // RENDERPASS_HPP