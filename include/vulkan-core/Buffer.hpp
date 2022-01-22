#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "Core.hpp"

namespace vkc {

    struct BufferCreateInfo {
        size_t alloc_size = 0;
        VkBufferUsageFlags usage = 0;
        VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_UNKNOWN;
    };

    struct AllocatedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    AllocatedBuffer create_buffer(vkc::Core& core, const BufferCreateInfo& buffer_create_info);
    AllocatedBuffer create_buffer_from_data(vkc::Core& core, const BufferCreateInfo& buffer_create_info, void* data);

    void destroy_buffer(vkc::Core& core, AllocatedBuffer buffer);
}

#endif // BUFFER_HPP