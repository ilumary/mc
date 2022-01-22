#ifndef SHADER_MODULE_HPP
#define SHADER_MODULE_HPP

#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <vector>

#include "Core.hpp"

namespace vkc {

class ShaderModule {
    VkDevice device_;
    VkShaderModule shader_module_;

public: 
    ShaderModule(Core& core, const std::string& filename);
    ~ShaderModule();

    inline VkShaderModule shader_module() const { return shader_module_; }

private:
    std::vector<char> readFile(const std::string& filename);
};

}

#endif // SHADER_MODULE_HPP