#ifndef TOXENGINE_H_
#define TOXENGINE_H_

#include "../App/App.h"
#include "Buffer.h"
#include "Context.h"
#include "Model.h"
#include "RTXModel.h"
#include "Sampler.h"
#include "SwapChain.h"
#include "Texture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

const uint32_t WIDTH = 1024;
const uint32_t HEIGHT = 1024;

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct RTUniformBufferObject {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class TOXEngine {
public:
  TOXEngine(App &app) : app(app) {}
  ~TOXEngine() {}

  void run();

  void loadModel(const std::string modelPath, const std::string texturePath);
  void loadRTXModel(const std::string path);

  App &app;
  Context context;

  std::unique_ptr<SwapChain> swapChain;

  // todo these should be vectors
  std::unique_ptr<Sampler> sampler;
  std::unique_ptr<Texture> texture;
  std::unique_ptr<Model> model;
  std::unique_ptr<RTXModel> rtx_model;

  float deltaTime;

private:
  void initVulkan();
  void mainLoop();
  void processInputs();

  float lastFrame;
};

#endif // TOXENGINE_H_
