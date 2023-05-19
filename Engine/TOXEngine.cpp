#include "TOXEngine.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void TOXEngine::run() {
  initVulkan();
  mainLoop();
}

void TOXEngine::initVulkan() {
  swapChain = std::make_shared<SwapChain>(context, this);
  sampler = std::make_shared<Sampler>(context);
  texture = std::make_shared<Texture>(context, TEXTURE_PATH);
  model = std::make_shared<Model>(context, MODEL_PATH);
  swapChain->createDescriptorSets();
}

void TOXEngine::mainLoop() {
  while (!glfwWindowShouldClose(context.window)) {
    glfwPollEvents();
    swapChain->drawFrame();
  }

  context.device->waitIdle();
}
