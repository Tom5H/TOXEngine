#ifndef TOXENGINE_ENGINE_RASTERIZER_H_
#define TOXENGINE_ENGINE_RASTERIZER_H_

#include "Buffer.h"
#include "Context.h"
#include "Image.h"

#include <cstdint>
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class TOXEngine;
class SwapChain;

class Rasterizer {
public:
  Rasterizer(Context &context, TOXEngine *engine, SwapChain *swapChain);

  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  std::vector<void *> uniformBuffersMapped;

  VkDescriptorPool descriptorPool;

  void createDescriptorSets();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                           uint32_t currentFrame);

  void refresh();

private:
  Context &context;
  TOXEngine *engine;
  SwapChain *swapChain;

  std::unique_ptr<Image> depthImage;

  std::vector<std::unique_ptr<Buffer>> uniformBuffers;

  std::vector<VkDescriptorSet> descriptorSets;

  void createRenderPass();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createDepthResources();
  void createUniformBuffers();
  void createDescriptorPool();
};

#endif // TOXENGINE_ENGINE_RASTERIZER_H_
