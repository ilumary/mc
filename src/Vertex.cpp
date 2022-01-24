#include "Vertex.hpp"

VkVertexInputBindingDescription Vertex::binding_description() {
    return VkVertexInputBindingDescription {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::vector<VkVertexInputAttributeDescription> Vertex::attributes_description() {
    std::vector<VkVertexInputAttributeDescription> attributes;
    
	VkVertexInputAttributeDescription positionAttribute = {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position),
    };

	VkVertexInputAttributeDescription normalAttribute = {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, normal),
    };

    VkVertexInputAttributeDescription uvAttribute = {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, uv),
    };

	attributes.push_back(positionAttribute);
	attributes.push_back(normalAttribute);
    attributes.push_back(uvAttribute);
	return attributes;
}