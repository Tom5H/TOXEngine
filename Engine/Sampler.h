#ifndef TOXENGINE_ENGINE_SAMPLER_H_
#define TOXENGINE_ENGINE_SAMPLER_H_

#include <vulkan/vulkan.h>

class Context;

class Sampler {
public:
  Sampler(Context &context);
  ~Sampler();

  VkSampler get() { return sampler; }

private:
  Context &context;

  VkSampler sampler;
};

#endif // TOXENGINE_ENGINE_SAMPLER_H_
