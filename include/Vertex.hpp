#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>


/**
 *  Represent a vertex for geometry with positional and normal vector
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    /**
	 * Get binding description for the vertex.
	 */
    static VkVertexInputBindingDescription binding_description();

    /**
	 * Get attrubute description to pass information to the shader using correct location.
	 */
    static std::vector<VkVertexInputAttributeDescription> attributes_description();
};

#endif