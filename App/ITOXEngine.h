#ifndef ITOXENGINE_H_
#define ITOXENGINE_H_

#include "IApp.h"

#include <glm/glm.hpp>

#include <string>

const uint32_t WINDOW_WIDTH = 1024;
const uint32_t WINDOW_HEIGHT = 1024;

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class ITOXEngine {
public:
  ITOXEngine(IApp &app) : app(app) {}
  ~ITOXEngine() = default;

  virtual void run() = 0;

  // currently only supported in IApp::start()   -------------
  virtual void loadModel(const std::string modelPath,
                         const std::string texturePath) = 0;
  virtual void loadRTXModel(const std::string path) = 0;
  // ---------------------------------------------------------

  IApp &app;
};

#endif // ITOXENGINE_H_
