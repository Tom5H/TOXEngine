#ifndef TOXENGINE_H_
#define TOXENGINE_H_

#include "../App/App.h"
#include "Buffer.h"
#include "Context.h"
#include "Device.h"
#include "Model.h"
#include "PhysicalDevice.h"
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
  ~TOXEngine() { cleanup(); }

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
  std::shared_ptr<Texture> texture;

  VkSampler textureSampler;

  std::shared_ptr<Model>
      model; // todo this should be an array of models (scene)
private:
  void initVulkan() {
    physicalDevice = std::make_shared<PhysicalDevice>(context);
    device = std::make_shared<Device>(physicalDevice);
    swapChain = std::make_shared<SwapChain>(this);
    texture = std::make_shared<Texture>(this, TEXTURE_PATH);
    createTextureSampler();
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

  void cleanup() {
    // cleanupSwapChain();

    // vkDestroyPipeline(device->get(), graphicsPipeline, nullptr);
    // vkDestroyPipelineLayout(device->get(), pipelineLayout, nullptr);
    // vkDestroyRenderPass(device->get(), renderPass, nullptr);

    /*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(device->get(), uniformBuffers[i], nullptr);
      vkFreeMemory(device->get(), uniformBuffersMemory[i], nullptr);
      }*/

    // vkDestroyDescriptorPool(device->get(), descriptorPool, nullptr);

    vkDestroySampler(device->get(), textureSampler, nullptr);
    //vkDestroyImageView(device->get(), textureImageView, nullptr);

    //vkDestroyImage(device->get(), textureImage, nullptr);
    //vkFreeMemory(device->get(), textureImageMemory, nullptr);

    // vkDestroyDescriptorSetLayout(device->get(), descriptorSetLayout,
    // nullptr);

    // vkDestroyBuffer(device->get(), indexBuffer, nullptr);
    // vkFreeMemory(device->get(), indexBufferMemory, nullptr);

    // vkDestroyBuffer(device->get(), vertexBuffer, nullptr);
    // vkFreeMemory(device->get(), vertexBufferMemory, nullptr);

    /*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device->get(), renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device->get(), imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(device->get(), inFlightFences[i], nullptr);
      }*/

    // vkDestroyCommandPool(device->get(), commandPool, nullptr);

    // vkDestroyDevice(device, nullptr)

    // if (enableValidationLayers) {
    //   DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    // }

    // vkDestroySurfaceKHR(instance, surface, nullptr);
    // vkDestroyInstance(instance, nullptr);

    // glfwDestroyWindow(window);

    // glfwTerminate();
  }

  void createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice->get(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device->get(), &samplerInfo, nullptr,
                        &textureSampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
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
