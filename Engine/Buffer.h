#ifndef TOXENGINE_BUFFER_H_
#define TOXENGINE_BUFFER_H_

#include <vulkan/vulkan.h>

#include <memory>

class Context;

class Buffer {
public:
  enum class Type {
    Scratch,
    Staging,
    Vertex,
    Index,
    Uniform,
    AccelInput,
    AccelStorage,
    ShaderBindingTable
  };

  Buffer(Context &context, Type type, VkDeviceSize size);
  ~Buffer();

  VkBuffer get() { return buffer; }
  VkDeviceMemory getDeviceMemory() { return memory; }
  VkDeviceAddress getDeviceAddress() { return deviceAddress; }

  void copy(Buffer other, VkDeviceSize size);

private:
  Context &context;
  VkBuffer buffer;
  VkDeviceMemory memory;
  VkDeviceAddress deviceAddress = 0;
};

#endif // TOXENGINE_BUFFER_H_
