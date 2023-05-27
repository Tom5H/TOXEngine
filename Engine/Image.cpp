#include "Image.h"

#include "TOXEngine.h"

Image::Image(Context &context, uint32_t width, uint32_t height, Type type)
    : context(context) {
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags properties;
  switch (type) {
  case Type::Depth:
    format = context.physicalDevice->findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    tiling = VK_IMAGE_TILING_OPTIMAL;
    usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Texture:
    format = VK_FORMAT_R8G8B8A8_SRGB;
    tiling = VK_IMAGE_TILING_OPTIMAL;
    usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::RTOutputImage:
    format = VK_FORMAT_B8G8R8A8_UNORM;
    tiling = VK_IMAGE_TILING_OPTIMAL;
    usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  }

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(context.device->get(), &imageInfo, nullptr, &image) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(context.device->get(), image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = context.physicalDevice->findMemoryType(
      memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(context.device->get(), &allocInfo, nullptr, &memory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(context.device->get(), image, memory, 0);
}

Image::~Image() {
  vkDestroyImage(context.device->get(), image, nullptr);
  vkFreeMemory(context.device->get(), memory, nullptr);
}

VkImageView Image::createImageView(VkImageAspectFlags aspectFlags) {
  return context.device->createImageView(image, format, aspectFlags);
}

void Image::transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout, bool raytracing) {
  VkCommandBuffer commandBuffer = context.device->beginSingleTimeCommands();

  transitionLayout(oldLayout, newLayout, commandBuffer, raytracing);

  context.device->endSingleTimeCommands(commandBuffer);
}

void Image::transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout,
                             VkCommandBuffer commandBuffer, bool raytracing) {
  context.device->transitionImageLayout(image, oldLayout, newLayout,
                                        commandBuffer, raytracing);
}

void Image::copyBuffer(VkBuffer buffer, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = context.device->beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  context.device->endSingleTimeCommands(commandBuffer);
}
