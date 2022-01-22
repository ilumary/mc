#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include <vulkan/vulkan.h>

#include "Core.hpp"

namespace vkc {

enum class PolygonMode {
    fill = VK_POLYGON_MODE_FILL,
    line = VK_POLYGON_MODE_LINE,
    point = VK_POLYGON_MODE_POINT,
};

enum class CullMode {
    none = VK_CULL_MODE_NONE,
    front = VK_CULL_MODE_FRONT_BIT,
    back = VK_CULL_MODE_BACK_BIT,
    front_and_back = VK_CULL_MODE_FRONT_AND_BACK,
};

struct GraphicsPipelineCreateInfo {
    // Required
    VkPipelineLayout pipeline_layout = {};
    VkRenderPass render_pass = {};
    VkExtent2D window_extent = {};

    // Optional
    VkPipelineShaderStageCreateInfo* shader_stages;
    uint32_t shader_stage_count = 0;
    PolygonMode polygon_mode = PolygonMode::fill;
    CullMode cull_mode = CullMode::none;
};

VkPipeline create_graphics_pipeline(Core& core, const GraphicsPipelineCreateInfo& create_info);

}

#endif // GRAPHICS_PIPELINE_HPP