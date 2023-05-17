#ifndef TOXENGINE_BUFFER_H_
#define TOXENGINE_BUFFER_H_

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class TOXEngine;

class Buffer {
public:
  enum class Type {
    Scratch,
    Staging,
    Vertex,
    Index,
    Uniform
  };
  
  Buffer(TOXEngine* engine, Type type, VkDeviceSize size);
  ~Buffer();

  VkBuffer get() { return buffer; }
  VkDeviceMemory getDeviceMemory() { return memory; }

  void copy(Buffer other, VkDeviceSize size);

protected:
  Buffer() = default;

private:
  TOXEngine *engine;
  VkBuffer buffer;
  VkDeviceMemory memory;
};

#endif // TOXENGINE_BUFFER_H_
