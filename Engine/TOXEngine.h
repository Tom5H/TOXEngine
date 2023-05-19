#ifndef TOXENGINE_H_
#define TOXENGINE_H_

#include "../App/App.h"
#include "Buffer.h"
#include "Context.h"
#include "Model.h"
#include "Sampler.h"
#include "SwapChain.h"
#include "Texture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../resources/models/viking_room.obj";
const std::string TEXTURE_PATH = "../resources/textures/viking_room.png";

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class TOXEngine {
public:
  TOXEngine() : app(this) {}
  ~TOXEngine() {}

  void run();

  App app;
  Context context;

  std::shared_ptr<SwapChain> swapChain;

  // todo these should be vectors
  std::shared_ptr<Sampler> sampler;
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Model> model;
  
private:
  void initVulkan();
  void mainLoop();
  };

#endif // TOXENGINE_H_
