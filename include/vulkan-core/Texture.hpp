#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <fmt/core.h>

#include "Core.hpp"
#include "Image.hpp"

class Texture {
public:
    vkc::AllocatedImage image_; 
    VkImageView image_view_;

    VkFilter magFilter = VK_FILTER_NEAREST;
    VkFilter minFilter = VK_FILTER_NEAREST;

    void createSampler(vkc::Core &core, VkSampler &textureSampler);
    void destroy(vkc::Core *core);
};

#endif // TEXTURE_HPP