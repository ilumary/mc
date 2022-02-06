#include "Mesh.hpp"

#include <iostream>

void Mesh::destroy(vkc::Core& core) {
    vmaDestroyBuffer(core.allocator(), index_buffer.buffer, index_buffer.allocation);
    vmaDestroyBuffer(core.allocator(), vertex_buffer.buffer, vertex_buffer.allocation);
}

void Mesh::merge(const Mesh* other) {
    //take vertex count before vertice merge to prevent corruption of first element
    uint32_t vertex_count = vertices.size();

    //std::cout << "Merging Mesh with " << other->vertices.size() << " into Mesh with "<< vertices.size() << " vertices" << std::endl;

    vertices.insert(vertices.end(), other->vertices.begin(), other->vertices.end());

    std::vector<uint32_t> tmp = other->indices;

    //std::cout << "Merging " << tmp.size() << " indices onto " << vertices.size() << " vertices" << std::endl;

    for(std::size_t i = 0; i < tmp.size(); ++i) {
        tmp[i] += vertex_count;
    }

    indices.insert(indices.end(), tmp.begin(), tmp.end());

    //std::cout << "Mesh now containing " << vertices.size() << " vertices and " << indices.size() << " indices" << std::endl;
}