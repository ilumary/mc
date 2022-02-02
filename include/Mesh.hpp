#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include "Vertex.hpp"
#include "Buffer.hpp"
#include "Core.hpp"

class Mesh {
public:
    //Vertex
	std::vector<Vertex> vertices;
	vkc::AllocatedBuffer vertex_buffer;

	//Indices
	std::vector<uint32_t> indices;
	vkc::AllocatedBuffer index_buffer;

	void generate();

	void merge(const Mesh* other);

	void destroy(vkc::Core& core);
};

#endif