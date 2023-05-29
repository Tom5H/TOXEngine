#include "Raytracer.h"

#include "Context.h"
#include "Shader.h"
#include "SwapChain.h"
#include "TOXEngine.h"

#include <array>
#include <cstdint>
#include <stdexcept>

Raytracer::Raytracer(Context &context, TOXEngine *engine, SwapChain *swapChain)
    : context(context), engine(engine), swapChain(swapChain) {
  createDescriptorSetLayout();
  createUniformBuffer();
  createDescriptorPool();
  createPipeline();
  createShaderBindingTable();
}

void Raytracer::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding asLayoutBinding{};
  asLayoutBinding.binding = 0;
  asLayoutBinding.descriptorCount = 1;
  asLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
  asLayoutBinding.pImmutableSamplers = nullptr;
  asLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

  VkDescriptorSetLayoutBinding storageImageLayoutBinding{};
  storageImageLayoutBinding.binding = 1;
  storageImageLayoutBinding.descriptorCount = 1;
  storageImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  storageImageLayoutBinding.pImmutableSamplers = nullptr;
  storageImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

  VkDescriptorSetLayoutBinding vertexBinding{};
  vertexBinding.binding = 2;
  vertexBinding.descriptorCount = 1;
  vertexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  vertexBinding.pImmutableSamplers = nullptr;
  vertexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

  VkDescriptorSetLayoutBinding indexBinding{};
  indexBinding.binding = 3;
  indexBinding.descriptorCount = 1;
  indexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  indexBinding.pImmutableSamplers = nullptr;
  indexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

  VkDescriptorSetLayoutBinding faceBinding{};
  faceBinding.binding = 4;
  faceBinding.descriptorCount = 1;
  faceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  faceBinding.pImmutableSamplers = nullptr;
  faceBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

  VkDescriptorSetLayoutBinding uniformBinding{};
  uniformBinding.binding = 5;
  uniformBinding.descriptorCount = 1;
  uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniformBinding.pImmutableSamplers = nullptr;
  uniformBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

  std::array<VkDescriptorSetLayoutBinding, 6> bindings = {
      asLayoutBinding, storageImageLayoutBinding,
      vertexBinding,   indexBinding,
      faceBinding,     uniformBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(context.device->get(), &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create RT descriptor set layout!");
  }
}

void Raytracer::createDescriptorPool() {
  std::array<VkDescriptorPoolSize, 6> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
  poolSizes[0].descriptorCount = 1; // maybe MAX_FRAMES_IN_FLIGHT
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  poolSizes[1].descriptorCount = 1;
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[2].descriptorCount = 1;
  poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[3].descriptorCount = 1;
  poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[4].descriptorCount = 1;
  poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[5].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 1; // --

  if (vkCreateDescriptorPool(context.device->get(), &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create RT descriptor pool!");
  }
}
void Raytracer::createDescriptorSet() {
  outputImage = std::make_unique<Image>(context, swapChain->getWidth(),
                                          swapChain->getHeight(),
                                          Image::Type::RTOutputImage);
  outputImage->transitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_GENERAL, true);
  outputImageView = outputImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptorSetLayout;

  if (vkAllocateDescriptorSets(context.device->get(), &allocInfo,
                               &descriptorSet) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate RT descriptor sets!");
  }

  VkAccelerationStructureKHR tlas = engine->rtx_model->TLAS->accel;

  VkWriteDescriptorSetAccelerationStructureKHR descASInfo{};
  descASInfo.sType =
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
  descASInfo.accelerationStructureCount = 1;
  descASInfo.pAccelerationStructures = &tlas;

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageView = outputImageView;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

  VkDescriptorBufferInfo vertexBufferInfo{};
  vertexBufferInfo.buffer = engine->rtx_model->vertexBuffer->get();
  vertexBufferInfo.range = VK_WHOLE_SIZE;

  VkDescriptorBufferInfo indexBufferInfo{};
  indexBufferInfo.buffer = engine->rtx_model->indexBuffer->get();
  indexBufferInfo.range = VK_WHOLE_SIZE;

  VkDescriptorBufferInfo faceBufferInfo{};
  faceBufferInfo.buffer = engine->rtx_model->faceBuffer->get();
  faceBufferInfo.range = VK_WHOLE_SIZE;

  VkDescriptorBufferInfo uniformBufferInfo{};
  uniformBufferInfo.buffer = uniformBuffer->get();
  uniformBufferInfo.range = sizeof(RTUniformBufferObject);

  std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = descriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType =
      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pNext = &descASInfo;

  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = descriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].dstArrayElement = 0;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].pImageInfo = &imageInfo;

  descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[2].dstSet = descriptorSet;
  descriptorWrites[2].dstBinding = 2;
  descriptorWrites[2].dstArrayElement = 0;
  descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[2].descriptorCount = 1;
  descriptorWrites[2].pBufferInfo = &vertexBufferInfo;

  descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[3].dstSet = descriptorSet;
  descriptorWrites[3].dstBinding = 3;
  descriptorWrites[3].dstArrayElement = 0;
  descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[3].descriptorCount = 1;
  descriptorWrites[3].pBufferInfo = &indexBufferInfo;

  descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[4].dstSet = descriptorSet;
  descriptorWrites[4].dstBinding = 4;
  descriptorWrites[4].dstArrayElement = 0;
  descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[4].descriptorCount = 1;
  descriptorWrites[4].pBufferInfo = &faceBufferInfo;

  descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[5].dstSet = descriptorSet;
  descriptorWrites[5].dstBinding = 5;
  descriptorWrites[5].dstArrayElement = 0;
  descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[5].descriptorCount = 1;
  descriptorWrites[5].pBufferInfo = &uniformBufferInfo;

  vkUpdateDescriptorSets(context.device->get(),
                         static_cast<uint32_t>(descriptorWrites.size()),
                         descriptorWrites.data(), 0, nullptr);
}

void Raytracer::createPipeline() {
  enum StageIndices { eRaygen, eMiss, eClosestHit, eShaderGroupCount };
  std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount> stages{};
  VkPipelineShaderStageCreateInfo stage{};
  stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage.pName = "main";
  // Raygen
  Shader raygen(context, "../resources/shaders/raytrace.rgen.spv");
  stage.module = raygen.get();
  stage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  stages[eRaygen] = stage;
  // Miss
  Shader miss(context, "../resources/shaders/raytrace.rmiss.spv");
  stage.module = miss.get();
  stage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
  stages[eMiss] = stage;
  // Closest Hit
  Shader chit(context, "../resources/shaders/raytrace.rchit.spv");
  stage.module = chit.get();
  stage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  stages[eClosestHit] = stage;

  VkRayTracingShaderGroupCreateInfoKHR group{};
  group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
  group.anyHitShader = VK_SHADER_UNUSED_KHR;
  group.closestHitShader = VK_SHADER_UNUSED_KHR;
  group.generalShader = VK_SHADER_UNUSED_KHR;
  group.intersectionShader = VK_SHADER_UNUSED_KHR;
  // Raygen
  group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  group.generalShader = eRaygen;
  shaderGroups.push_back(group);
  // Miss
  group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  group.generalShader = eMiss;
  shaderGroups.push_back(group);
  // Closest Hit
  group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
  group.generalShader = VK_SHADER_UNUSED_KHR;
  group.closestHitShader = eClosestHit;
  shaderGroups.push_back(group);

  std::array<VkPushConstantRange, 2> pushRanges{};

  pushRanges[0].size = sizeof(int);
  pushRanges[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  pushRanges[1].size = sizeof(int);
  pushRanges[1].offset = sizeof(int);
  pushRanges[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount = 1;
  pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
  pipelineLayoutCreateInfo.pushConstantRangeCount = pushRanges.size();
  pipelineLayoutCreateInfo.pPushConstantRanges = pushRanges.data();

  vkCreatePipelineLayout(context.device->get(), &pipelineLayoutCreateInfo,
                         nullptr, &pipelineLayout);

  VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{};
  rayPipelineInfo.sType =
      VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
  rayPipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
  rayPipelineInfo.pStages = stages.data();
  rayPipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
  rayPipelineInfo.pGroups = shaderGroups.data();
  rayPipelineInfo.maxPipelineRayRecursionDepth = 2;
  rayPipelineInfo.layout = pipelineLayout;

  vkCreateRayTracingPipelinesKHR(context.device->get(), {}, {}, 1,
                                 &rayPipelineInfo, nullptr, &pipeline);
}

void Raytracer::createShaderBindingTable() {
  uint32_t missCount{1};
  uint32_t hitCount{1};
  uint32_t handleCount = 1 + missCount + hitCount;
  VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties{};
  rtProperties.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
  VkPhysicalDeviceProperties2 props2{};
  props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
  props2.pNext = &rtProperties;
  vkGetPhysicalDeviceProperties2(context.physicalDevice->get(), &props2);
  uint32_t handleSize = rtProperties.shaderGroupHandleSize;
  uint32_t handleSizeAligned = rtProperties.shaderGroupHandleAlignment;
  uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
  uint32_t sbtSize = groupCount * handleSizeAligned;

  std::vector<uint8_t> handleStorage(sbtSize);
  if (vkGetRayTracingShaderGroupHandlesKHR(
          context.device->get(), pipeline, 0, groupCount, sbtSize,
          handleStorage.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to get RT SBT handles!");
  }

  raygenSBT =
      std::make_unique<Buffer>(context, Buffer::Type::ShaderBindingTable,
                               handleSize, handleStorage.data());
  missSBT = std::make_unique<Buffer>(context, Buffer::Type::ShaderBindingTable,
                                     handleSize,
                                     handleStorage.data() + handleSizeAligned);
  hitSBT = std::make_unique<Buffer>(
      context, Buffer::Type::ShaderBindingTable, handleSize,
      handleStorage.data() + 2 * handleSizeAligned);

  uint32_t stride = rtProperties.shaderGroupHandleAlignment;
  uint32_t size = rtProperties.shaderGroupHandleAlignment;

  raygenRegion.deviceAddress = raygenSBT->getDeviceAddress();
  raygenRegion.size = size;
  raygenRegion.stride = stride;

  missRegion.deviceAddress = missSBT->getDeviceAddress();
  missRegion.size = size;
  missRegion.stride = stride;

  hitRegion.deviceAddress = hitSBT->getDeviceAddress();
  hitRegion.size = size;
  hitRegion.stride = stride;
}

void Raytracer::createUniformBuffer() {
  VkDeviceSize bufferSize = sizeof(RTUniformBufferObject);

  uniformBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Uniform, bufferSize);

  vkMapMemory(context.device->get(), uniformBuffer->getDeviceMemory(), 0,
              bufferSize, 0, &uniformBufferMapped);
}

void Raytracer::recordCommandBuffer(const VkCommandBuffer &commandBuffer) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                    pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                          pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  vkCmdPushConstants(commandBuffer, pipelineLayout,
                     VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(int), &frame);
  if (context.camera.getHasMoved()) {
    standingFrames = 0;
  }
  vkCmdPushConstants(commandBuffer, pipelineLayout,
                     VK_SHADER_STAGE_RAYGEN_BIT_KHR, sizeof(int), sizeof(int),
                     &standingFrames);
  vkCmdTraceRaysKHR(commandBuffer, &raygenRegion, &missRegion, &hitRegion,
                    &callRegion, swapChain->getWidth(), swapChain->getHeight(),
                    2);
  swapChain->copyToBackImage(*outputImage);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
  context.device->waitIdle();
  frame++;
  standingFrames++;
}
