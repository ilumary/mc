#include "Buffer.hpp"

namespace vkc {
    AllocatedBuffer create_buffer(vkc::Core& core, const BufferCreateInfo& buffer_create_info) {
        VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .size = buffer_create_info.alloc_size,
            .usage = buffer_create_info.usage,
        };

        VmaAllocationCreateInfo vmaalloc_info = {.usage = buffer_create_info.memory_usage,};

        AllocatedBuffer new_buffer;

        vmaCreateBuffer(core.allocator(), &buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation, nullptr);

        return new_buffer;
    }

    AllocatedBuffer create_buffer_from_data(vkc::Core& core, const BufferCreateInfo& buffer_create_info, void* data) {
        AllocatedBuffer buffer = create_buffer(core, buffer_create_info);
        void* mapped_mem = nullptr;
        vmaMapMemory(core.allocator(), buffer.allocation, &mapped_mem);
        memcpy(mapped_mem, data, buffer_create_info.alloc_size);
        vmaUnmapMemory(core.allocator(), buffer.allocation);
        return buffer; 
    }

    void destroy_buffer(vkc::Core& core, AllocatedBuffer buffer) {
        vmaDestroyBuffer(core.allocator(), buffer.buffer, buffer.allocation);
    }
}