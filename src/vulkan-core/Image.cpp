#include "Image.hpp"

void vkc::create_image(vkc::Core& core, const ImageCreateInfo& image_create_info, AllocatedImage& image) {
	VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = image_create_info.tex_width,
        .extent.height = image_create_info.tex_height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = image_create_info.format,
        .tiling = image_create_info.image_tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = image_create_info.usage_flags,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo img_allocinfo = { .usage = VMA_MEMORY_USAGE_GPU_ONLY, };

    vmaCreateImage(core.allocator(), &imageInfo, &img_allocinfo, &image.image, &image.allocation, nullptr);

	/*Allocate memory for the texture
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(core.device(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties),
    };

	if (vkAllocateMemory(device.logical, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("vkCraft: Failed to allocate image memory!");
	}

	vkBindImageMemory(device.logical, image, imageMemory, 0);*/
}

void vkc::create_image_view(vkc::Core& core, const ImageViewCreateInfo& image_view_create_info, VkImageView& image_view) {
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image_view_create_info.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = image_view_create_info.format,
        .subresourceRange.aspectMask = image_view_create_info.aspect_flags,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

	if (vkCreateImageView(core.device(), &view_info, nullptr, &image_view) != VK_SUCCESS){
		fmt::print("vkCraft: failed to create texture image view!");
	}
}