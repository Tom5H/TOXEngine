#include "Texture.h"

#include "Image.h"
#include "PhysicalDevice.h"
#include "TOXEngine.h"

#include <stb_image.h>

#include <cstring>

Texture::Texture(Context &context, const std::string path) : context(context) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  Buffer stagingBuffer(context, Buffer::Type::Staging, imageSize);

  void *data;
  vkMapMemory(context.device->get(), stagingBuffer.getDeviceMemory(), 0,
              imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(context.device->get(), stagingBuffer.getDeviceMemory());

  stbi_image_free(pixels);

  image = std::make_shared<Image>(context, texWidth, texHeight,
                                  Image::Type::Texture);
  image->transitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  image->copyBuffer(stagingBuffer.get(), static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  imageView = image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
}

Texture::~Texture() {
  vkDestroyImageView(context.device->get(), imageView, nullptr);
}
