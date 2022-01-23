#include "Mesh.hpp"

void Mesh::destroy(vkc::Core& core) {
    vmaDestroyBuffer(core.allocator(), index_buffer.buffer, index_buffer.allocation);
    vmaDestroyBuffer(core.allocator(), vertex_buffer.buffer, vertex_buffer.allocation);
}