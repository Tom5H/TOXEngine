#include "TOXEngine.h"
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
  texture = std::make_unique<Texture>(context, TEXTURE_PATH);
  // TODO interface function to load models for application (in startup first)
  // TODO factory selecting Model or RTXModel
  model = std::make_unique<Model>(context, MODEL_PATH);
  rtx_model = std::make_unique<RTXModel>(context, RTX_MODEL_PATH);
  swapChain->createDescriptorSets();
  swapChain->createRTDescriptorSet();
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
