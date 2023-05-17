#ifndef TOXENGINE_SHADER_H_
#define TOXENGINE_SHADER_H_

#include <vulkan/vulkan.h>

#include <string>

class TOXEngine;

class Shader {
public:
  Shader(TOXEngine *engine, const std::string path);
  ~Shader();

  VkShaderModule get() { return shader; }

private:
  TOXEngine *engine;
  VkShaderModule shader;
};

#endif // TOXENGINE_SHADER_H_
