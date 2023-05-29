#ifndef ITOXENGINE_H_
#define ITOXENGINE_H_

#include "App.h"

#include <glm/glm.hpp>

#include <string>

const uint32_t WIDTH = 1024;
const uint32_t HEIGHT = 1024;

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class ITOXEngine {
public:
  ITOXEngine(App &app) : app(app) {}
  ~ITOXEngine() = default;

  virtual void run() = 0;

  virtual void loadModel(const std::string modelPath,
                         const std::string texturePath) = 0;
  virtual void loadRTXModel(const std::string path) = 0;

  App &app;
};

#endif // ITOXENGINE_H_
