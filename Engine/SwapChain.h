#ifndef TOXENGINE_SWAPCHAIN_H_
#define TOXENGINE_SWAPCHAIN_H_

#include "Buffer.h"
#include "Image.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <vector>

class Context;
class TOXEngine;

class SwapChain {
public:
  SwapChain(Context &context, TOXEngine *engine);
  ~SwapChain();

  VkSwapchainKHR get() { return swapChain; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }
  VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

  void create();
  void cleanup();
  void recreate();

  void drawFrame();
  void createDescriptorSets();
  void createRTDescriptorSet();
  void copyToBackImage(Image &image);

  bool useRaytracer = true;

private:
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createDepthResources();
  void createFramebuffers();
  void createUniformBuffers();
  void createDescriptorPool();
  void createCommandBuffers();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
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

  // rasterizer

  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  std::unique_ptr<Image> depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  std::vector<std::unique_ptr<Buffer>> uniformBuffers;
  std::vector<void *> uniformBuffersMapped;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

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
