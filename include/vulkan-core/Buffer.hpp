#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <string.h>

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

    VkCommandBuffer begin_single_time_commands(vkc::Core& core, VkCommandPool& command_pool);
    void submit_single_time_commands(vkc::Core& core, VkCommandPool& command_pool, VkCommandBuffer& command_buffer);

    void destroy_buffer(vkc::Core& core, AllocatedBuffer buffer);
}

#endif // BUFFER_HPP
