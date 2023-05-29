#ifndef TOXENGINE_SWAPCHAIN_H_
#define TOXENGINE_SWAPCHAIN_H_

#include "Buffer.h"
#include "Image.h"
#include "Rasterizer.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class Context;
class TOXEngine;

class SwapChain {
public:
  SwapChain(Context &context, TOXEngine *engine);
  ~SwapChain();

  VkSwapchainKHR get() { return swapChain; }
  VkExtent2D getExtent() { return swapChainExtent; }
  uint32_t getWidth() { return swapChainExtent.width; }
  uint32_t getHeight() { return swapChainExtent.height; }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkFramebuffer getFramebuffer(uint32_t index) {
    return swapChainFramebuffers[index];
  }

  void refresh();

  void drawFrame();

  void createRTDescriptorSet();
  void copyToBackImage(Image &image);

  bool useRaytracer = true;

private:
  void create();
  void cleanup();
  void recreate();

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  void createImageViews();
  void createFramebuffers();
  void createCommandBuffers();
  void createSyncObjects();

  void createRTDescriptorSetLayout();
  void createRTUniformBuffer();
  void createRTDescriptorPool();
  void createRTPipeline();
  void createRTShaderBindingTable();
  void raytrace(const VkCommandBuffer &commandBuffer);

  Context &context;
  TOXEngine *engine;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::unique_ptr<Rasterizer> rasterizer;

  // raytracer

  VkDescriptorPool rtDescriptorPool;
  VkDescriptorSetLayout rtDescriptorSetLayout;
  VkDescriptorSet rtDescriptorSet;

  std::unique_ptr<Image> rtOutputImage;
  VkImageView rtOutputImageView;

  std::unique_ptr<Buffer> rtUniformBuffer;
  void *rtUniformBufferMapped;

  std::vector<VkRayTracingShaderGroupCreateInfoKHR> rtShaderGroups;
  VkPipelineLayout rtPipelineLayout;
  VkPipeline rtPipeline;

  std::unique_ptr<Buffer> raygenSBT;
  std::unique_ptr<Buffer> missSBT;
  std::unique_ptr<Buffer> hitSBT;
  VkStridedDeviceAddressRegionKHR raygenRegion{};
  VkStridedDeviceAddressRegionKHR missRegion{};
  VkStridedDeviceAddressRegionKHR hitRegion{};
  VkStridedDeviceAddressRegionKHR callRegion{};

  // general

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  uint32_t currentFrame = 0;
  uint32_t frame = 0;
  uint32_t standingFrames = 0;
  bool vsync = false;
};

#endif // TOXENGINE_SWAPCHAIN_H_
