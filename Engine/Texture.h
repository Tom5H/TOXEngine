#ifndef TOXENGINE_TEXTURE_H_
#define TOXENGINE_TEXTURE_H_

#include "Image.h"

#include <memory>
#include <string>

class TOXEngine;

class Texture {
public:
  Texture(TOXEngine *engine, const std::string path);
  ~Texture();

  VkImageView getImageView() { return imageView; }
  
private:
  TOXEngine *engine;
  std::shared_ptr<Image> image;
  VkImageView imageView;
};

#endif // TOXENGINE_TEXTURE_H_
