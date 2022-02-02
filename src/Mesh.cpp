#include "Mesh.hpp"

void Mesh::destroy(vkc::Core& core) {
    vmaDestroyBuffer(core.allocator(), index_buffer.buffer, index_buffer.allocation);
    vmaDestroyBuffer(core.allocator(), vertex_buffer.buffer, vertex_buffer.allocation);
}

void Mesh::merge(const Mesh* other) {
    for(std::size_t i = 0; i < other->vertices.size(); ++i) {
        vertices.push_back(other->vertices[i]);
    }

    uint32_t indice_count = indices.size();
    for(std::size_t i = 0; i < other->indices.size(); ++i) {
        indices.push_back(other->indices[i] + indice_count);
    }
    
}