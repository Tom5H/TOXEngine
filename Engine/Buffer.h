#ifndef TOXENGINE_ENGINE_BUFFER_H_
#define TOXENGINE_ENGINE_BUFFER_H_

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
    Face,
    Uniform,
    AccelInput,
    AccelStorage,
    ShaderBindingTable
  };

  Buffer(Context &context, Type type, VkDeviceSize size,
         const void *data = nullptr);
  ~Buffer();

  VkBuffer get() { return buffer; }
  VkDeviceMemory getDeviceMemory() { return memory; }
  VkDeviceAddress getDeviceAddress() { return deviceAddress; }

  void copy(Buffer other, VkDeviceSize size);
  void store(void *data, VkDeviceSize size);

  void cleanup();

private:
  Context &context;

  VkBuffer buffer;
  VkDeviceMemory memory;
  VkDeviceAddress deviceAddress = 0;

  bool staging = false; // staging buffers need to be explicitly cleaned
                        // otherwise there are validation layer complaints
};

#endif // TOXENGINE_ENGINE_BUFFER_H_
