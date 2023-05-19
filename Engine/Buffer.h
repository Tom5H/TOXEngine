#ifndef TOXENGINE_BUFFER_H_
#define TOXENGINE_BUFFER_H_

#include <vulkan/vulkan.h>

class Context;

class Buffer {
public:
  enum class Type {
    Scratch,
    Staging,
    Vertex,
    Index,
    Uniform
  };
  
  Buffer(Context &context, Type type, VkDeviceSize size);
  ~Buffer();

  VkBuffer get() { return buffer; }
  VkDeviceMemory getDeviceMemory() { return memory; }

  void copy(Buffer other, VkDeviceSize size);

protected:
  Buffer() = default;

private:
  Context &context;
  VkBuffer buffer;
  VkDeviceMemory memory;
};

#endif // TOXENGINE_BUFFER_H_
