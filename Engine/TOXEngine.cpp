#include "TOXEngine.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void TOXEngine::run() {
  initVulkan();
  mainLoop();
}

void TOXEngine::initVulkan() {
  physicalDevice = std::make_shared<PhysicalDevice>(context);
  device = std::make_shared<Device>(physicalDevice);
  swapChain = std::make_shared<SwapChain>(this);
  sampler = std::make_shared<Sampler>(this);
  texture = std::make_shared<Texture>(this, TEXTURE_PATH);
  model = std::make_shared<Model>(this, MODEL_PATH);
  swapChain->createDescriptorSets();
}

void TOXEngine::mainLoop() {
  while (!glfwWindowShouldClose(context.window)) {
    glfwPollEvents();
    swapChain->drawFrame();
  }

  device->waitIdle();
}
