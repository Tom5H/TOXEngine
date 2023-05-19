#ifndef TOXENGINE_SHADER_H_
#define TOXENGINE_SHADER_H_

#include <vulkan/vulkan.h>

#include <string>

class Context;

class Shader {
public:
  Shader(Context &context, const std::string path);
  ~Shader();

  VkShaderModule get() { return shader; }

private:
  Context &context;
  VkShaderModule shader;
};

#endif // TOXENGINE_SHADER_H_
