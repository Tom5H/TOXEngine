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

  Context &context;
  TOXEngine *engine;
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  std::shared_ptr<Image> depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  std::vector<std::shared_ptr<Buffer>> uniformBuffers;
  std::vector<void *> uniformBuffersMapped;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  uint32_t currentFrame = 0;
};

#endif // TOXENGINE_SWAPCHAIN_H_
