#ifndef TOXENGINE_ENGINE_RAYTRACER_H_
#define TOXENGINE_ENGINE_RAYTRACER_H_

#include "Buffer.h"
#include "Context.h"
#include "Image.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class TOXEngine;
class SwapChain;

class Raytracer {
public:
  Raytracer(Context &context, TOXEngine *engine, SwapChain *swapChain);

  void createDescriptorSet();
  void recordCommandBuffer(const VkCommandBuffer &commandBuffer);

  VkDescriptorPool descriptorPool;
  VkDescriptorSetLayout descriptorSetLayout;

  VkImageView outputImageView;

  void *uniformBufferMapped;

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

private:
  void createDescriptorSetLayout();
  void createDescriptorPool();
  void createPipeline();
  void createShaderBindingTable();
  void createUniformBuffer();

  Context &context;
  TOXEngine *engine;
  SwapChain *swapChain;

  VkDescriptorSet descriptorSet;

  std::unique_ptr<Image> outputImage;

  std::unique_ptr<Buffer> uniformBuffer;

  std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

  std::unique_ptr<Buffer> raygenSBT;
  std::unique_ptr<Buffer> missSBT;
  std::unique_ptr<Buffer> hitSBT;
  VkStridedDeviceAddressRegionKHR raygenRegion{};
  VkStridedDeviceAddressRegionKHR missRegion{};
  VkStridedDeviceAddressRegionKHR hitRegion{};
  VkStridedDeviceAddressRegionKHR callRegion{};

  uint32_t frame = 0;
  uint32_t standingFrames = 0;
};

#endif // TOXENGINE_ENGINE_RAYTRACER_H_
