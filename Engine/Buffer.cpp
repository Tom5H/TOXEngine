#include "Buffer.h"

#include "TOXEngine.h"

#include <cstring>

Buffer::Buffer(Context &context, Type type, VkDeviceSize size, const void *data)
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
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Index:
    usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Face:
    usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::Uniform:
    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
  case Type::AccelInput:
    usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
  case Type::AccelStorage:
    usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    break;
  case Type::ShaderBindingTable:
    usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
  }
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(context.device->get(), &bufferInfo, nullptr, &buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryAllocateFlagsInfo flagsInfo = {};
  flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context.device->get(), buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = context.physicalDevice->findMemoryType(
      memRequirements.memoryTypeBits, properties);
  allocInfo.pNext = &flagsInfo;

  if (vkAllocateMemory(context.device->get(), &allocInfo, nullptr, &memory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(context.device->get(), buffer, memory, 0);

  if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    VkBufferDeviceAddressInfo bufferDeviceAddressInfo;
    bufferDeviceAddressInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = buffer;
    bufferDeviceAddressInfo.pNext = NULL;
    deviceAddress = vkGetBufferDeviceAddress(context.device->get(),
                                             &bufferDeviceAddressInfo);
  }

  if (data) {
    void *mapped;
    vkMapMemory(context.device->get(), memory, 0, size, 0, &mapped);
    memcpy(mapped, data, size);
    vkUnmapMemory(context.device->get(), memory);
  }
}

Buffer::~Buffer() {
  //vkDestroyBuffer(context.device->get(), buffer, nullptr);
  //vkFreeMemory(context.device->get(), memory, nullptr);
}

void Buffer::copy(Buffer other, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = context.device->beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, other.buffer, buffer, 1, &copyRegion);

  context.device->endSingleTimeCommands(commandBuffer);
}
