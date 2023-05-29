#include "TOXEngine.h"
#include "Texture.h"
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void TOXEngine::run() {
  initVulkan();
  mainLoop();
}

void TOXEngine::initVulkan() {
  swapChain = std::make_unique<SwapChain>(context, this);
  sampler = std::make_unique<Sampler>(context);
  app.start();
  swapChain->refresh();
}

void TOXEngine::mainLoop() {
  while (!glfwWindowShouldClose(context.window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //std::cout << "fps: " << 1 / deltaTime << std::endl;

    glfwPollEvents();
    processInputs();
    swapChain->drawFrame();
  }

  context.device->waitIdle();
}

void TOXEngine::processInputs() {
  static bool r = true;
  if (glfwGetKey(context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  if (glfwGetKey(context.window, GLFW_KEY_W) == GLFW_PRESS)
    context.camera.ProcessKeyboard(Camera::Direction::Forward, deltaTime);
  if (glfwGetKey(context.window, GLFW_KEY_S) == GLFW_PRESS)
    context.camera.ProcessKeyboard(Camera::Direction::Backward, deltaTime);
  if (glfwGetKey(context.window, GLFW_KEY_A) == GLFW_PRESS)
    context.camera.ProcessKeyboard(Camera::Direction::Left, deltaTime);
  if (glfwGetKey(context.window, GLFW_KEY_D) == GLFW_PRESS)
    context.camera.ProcessKeyboard(Camera::Direction::Right, deltaTime);
  if (glfwGetKey(context.window, GLFW_KEY_Q) == GLFW_PRESS)
    context.camera.ProcessKeyboard(Camera::Direction::Up, deltaTime);
  if (glfwGetKey(context.window, GLFW_KEY_E) == GLFW_PRESS)
    context.camera.ProcessKeyboard(Camera::Direction::Down, deltaTime);

  if (glfwGetKey(context.window, GLFW_KEY_R) == GLFW_PRESS) {
    if (r) {
      r = false;
      swapChain->useRaytracer = !swapChain->useRaytracer;
    }
  } else {
    r = true;
  }
}

// todo vectors of models and textures -> push back
void TOXEngine::loadModel(const std::string modelPath,
                          const std::string texturePath) {
  texture = std::make_unique<Texture>(context, texturePath);
  model = std::make_unique<Model>(context, modelPath);
}

// todo vector of models -> push back
void TOXEngine::loadRTXModel(const std::string path) {
  rtx_model = std::make_unique<RTXModel>(context, path);
}
