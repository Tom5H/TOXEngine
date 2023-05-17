#ifndef TOXENGINE_SAMPLER_H_
#define TOXENGINE_SAMPLER_H_

#include <vulkan/vulkan.h>

class TOXEngine;

class Sampler {
public:
  Sampler(TOXEngine *engine);
  ~Sampler();

  VkSampler get() { return sampler; }

private:
  TOXEngine *engine;
  VkSampler sampler;
};

#endif // TOXENGINE_SAMPLER_H_
