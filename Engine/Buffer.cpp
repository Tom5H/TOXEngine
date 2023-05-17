#include "Buffer.h"

#include "TOXEngine.h"
#include <vulkan/vulkan_core.h>

Buffer::Buffer(TOXEngine *engine, Type type, VkDeviceSize size)
    : engine(engine) {
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

  if (vkCreateBuffer(engine->getDevice()->get(), &bufferInfo, nullptr,
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(engine->getDevice()->get(), buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = engine->getPhysicalDevice()->findMemoryType(
      memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(engine->getDevice()->get(), &allocInfo, nullptr,
                       &memory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(engine->getDevice()->get(), buffer, memory, 0);
}

Buffer::~Buffer() {
  vkDestroyBuffer(engine->getDevice()->get(), buffer, nullptr);
  vkFreeMemory(engine->getDevice()->get(), memory, nullptr);
}

void Buffer::copy(Buffer other, VkDeviceSize size) {
  VkCommandBuffer commandBuffer =
      engine->getDevice()->beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, other.buffer, buffer, 1, &copyRegion);

  engine->getDevice()->endSingleTimeCommands(commandBuffer);
}
