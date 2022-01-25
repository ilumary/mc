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

    VkCommandBuffer begin_single_time_commands(vkc::Core& core, VkCommandPool& command_pool) {
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandPool = command_pool,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(core.device(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void submit_single_time_commands(vkc::Core& core, VkCommandPool& command_pool, VkCommandBuffer& command_buffer) {
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;

        vkQueueSubmit(core.graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(core.graphics_queue());

        vkFreeCommandBuffers(core.device(), command_pool, 1, &command_buffer);
    }
}