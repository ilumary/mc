#include "Mesh.hpp"

#include <iostream>

void Mesh::destroy(vkc::Core& core) {
    vmaDestroyBuffer(core.allocator(), index_buffer.buffer, index_buffer.allocation);
    vmaDestroyBuffer(core.allocator(), vertex_buffer.buffer, vertex_buffer.allocation);
}

void Mesh::merge(const Mesh* other) {
    std::cout << "Merging Mesh with " << other->vertices.size() << " other vertices." << std::endl;

    vertices.insert(vertices.end(), other->vertices.begin(), other->vertices.end());

    std::cout << "Successfully merged vertices " << std::endl;

    std::vector<uint32_t> tmp = other->indices;
    uint32_t vertex_count = vertices.size();
    for(std::size_t i = 0; i < tmp.size(); ++i) {
        tmp[i] += vertex_count;
    }

    indices.insert(indices.end(), tmp.begin(), tmp.end());
}