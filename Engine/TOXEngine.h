#ifndef TOXENGINE_H_
#define TOXENGINE_H_

#include "../App/ITOXEngine.h"

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

struct RTUniformBufferObject {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class TOXEngine : public ITOXEngine {
public:
  TOXEngine(IApp &app) : ITOXEngine(app) {}
  ~TOXEngine() {}

  void run() override;

  void loadModel(const std::string modelPath, const std::string texturePath) override;
  void loadRTXModel(const std::string path) override;

  //App &app;
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
