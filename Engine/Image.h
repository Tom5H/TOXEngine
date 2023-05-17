#ifndef TOXENGINE_IMAGE_H_
#define TOXENGINE_IMAGE_H_

#include <vulkan/vulkan.h>

#include <cstdint>

class TOXEngine;

class Image {
public:
  enum class Type { Depth, Texture };

  Image(TOXEngine *engine, uint32_t width, uint32_t height, Type type);
  ~Image();

  VkImage get() { return image; }
  VkFormat getFormat() { return format; }
  VkImageView createImageView(VkImageAspectFlags aspectFlags);
  void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
  void copyBuffer(VkBuffer buffer, uint32_t width, uint32_t height);

private:
  TOXEngine *engine;
  VkImage image;
  VkDeviceMemory memory;
  VkFormat format;
};

#endif
