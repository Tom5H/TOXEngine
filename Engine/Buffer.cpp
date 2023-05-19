#include "Buffer.h"

#include "TOXEngine.h"

Buffer::Buffer(Context &context, Type type, VkDeviceSize size)
    : context(context) {
  VkBufferUsageFlags usage;
  VkMemoryPropertyFlags properties;
  switch (type) {
  case Type::Scratch:
    usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Staging:
    usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
  case Type::Vertex:
    usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Index:
    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Uniform:
    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
  }
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(context.device->get(), &bufferInfo, nullptr,
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context.device->get(), buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = context.physicalDevice->findMemoryType(
      memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(context.device->get(), &allocInfo, nullptr,
                       &memory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(context.device->get(), buffer, memory, 0);
}

Buffer::~Buffer() {
  vkDestroyBuffer(context.device->get(), buffer, nullptr);
  vkFreeMemory(context.device->get(), memory, nullptr);
}

void Buffer::copy(Buffer other, VkDeviceSize size) {
  VkCommandBuffer commandBuffer =
      context.device->beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, other.buffer, buffer, 1, &copyRegion);

  context.device->endSingleTimeCommands(commandBuffer);
}
