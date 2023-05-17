#ifndef TOXENGINE_H_
#define TOXENGINE_H_

#include "../App/App.h"
#include "Buffer.h"
#include "Context.h"
#include "Device.h"
#include "Model.h"
#include "PhysicalDevice.h"
#include "Sampler.h"
#include "SwapChain.h"
#include "Texture.h"
#include "Vertex.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../resources/models/viking_room.obj";
const std::string TEXTURE_PATH = "../resources/textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class TOXEngine {
public:
  TOXEngine() : app(this) {}
  ~TOXEngine() {}

  App app;
  Context context;

  void run() {
    initVulkan();
    mainLoop();
  }

  std::shared_ptr<PhysicalDevice> getPhysicalDevice() { return physicalDevice; }
  std::shared_ptr<Device> getDevice() { return device; }

private:
  std::shared_ptr<PhysicalDevice> physicalDevice;
  std::shared_ptr<Device> device;

  std::shared_ptr<SwapChain> swapChain;

public:

  // todo these should be vectors
  std::shared_ptr<Sampler> sampler;
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Model> model;
  
private:
  void initVulkan() {
    physicalDevice = std::make_shared<PhysicalDevice>(context);
    device = std::make_shared<Device>(physicalDevice);
    swapChain = std::make_shared<SwapChain>(this);
    sampler = std::make_shared<Sampler>(this);
    texture = std::make_shared<Texture>(this, TEXTURE_PATH);
    model = std::make_shared<Model>(this, MODEL_PATH);
    swapChain->createDescriptorSets();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(context.window)) {
      glfwPollEvents();
      swapChain->drawFrame();
    }

    device->waitIdle();
  }

public:
  VkShaderModule createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->get(), &createInfo, nullptr,
                             &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

private:
};

#endif // TOXENGINE_H_
