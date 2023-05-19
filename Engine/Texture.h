#ifndef TOXENGINE_TEXTURE_H_
#define TOXENGINE_TEXTURE_H_

#include "Image.h"

#include <memory>
#include <string>

class Context;

class Texture {
public:
  Texture(Context &context, const std::string path);
  ~Texture();

  VkImageView getImageView() { return imageView; }
  
private:
  Context &context;
  std::shared_ptr<Image> image;
  VkImageView imageView;
};

#endif // TOXENGINE_TEXTURE_H_
