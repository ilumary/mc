#include "ShaderModule.hpp"

#include <fstream>

namespace vkc {

    ShaderModule::ShaderModule(Core& core, const std::string& filename) : device_{core.device()}{
        auto code = readFile(filename);

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*> (code.data ());

        if(vkCreateShaderModule(core.device(), &create_info, nullptr, &shader_module_) != VK_SUCCESS) {
            fmt::print("Fatal Error creating shader module"); 
        }
    }

    ShaderModule::~ShaderModule() {
        vkDestroyShaderModule(device_, shader_module_, nullptr);
    }

    std::vector<char> ShaderModule::readFile(const std::string& filename) {
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

}