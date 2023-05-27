#ifndef TOXENGINE_IMAGE_H_
#define TOXENGINE_IMAGE_H_

#include <vulkan/vulkan.h>

#include <cstdint>

class Context;

class Image {
public:
  enum class Type { Depth, Texture, RTOutputImage };

  Image(Context &context, uint32_t width, uint32_t height, Type type);
  ~Image();

  VkImage get() { return image; }
  VkFormat getFormat() { return format; }
  VkImageView createImageView(VkImageAspectFlags aspectFlags);
  void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout, bool raytracing = false);
  void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer, bool raytracing = false);
  void copyBuffer(VkBuffer buffer, uint32_t width, uint32_t height);

private:
  Context &context;
  VkImage image;
  VkDeviceMemory memory;
  VkFormat format;
};

#endif
