#include "SwapChain.h"

#include "Buffer.h"
#include "Image.h"
#include "Rasterizer.h"
#include "Shader.h"
#include "TOXEngine.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

SwapChain::SwapChain(Context &context, TOXEngine *engine)
    : context(context), engine(engine) {
  create();
  createImageViews();

  std::cout << "#########" << std::endl;

  rasterizer = std::make_unique<Rasterizer>(context, engine, this);

  std::cout << "#########" << std::endl;

  createFramebuffers();
  createSyncObjects();
  createCommandBuffers();

  createRTDescriptorSetLayout();
  createRTUniformBuffer();
  createRTDescriptorPool();
  createRTPipeline();
  createRTShaderBindingTable();
}

SwapChain::~SwapChain() {
  cleanup();

  vkDestroyPipeline(context.device->get(), rtPipeline, nullptr);
  vkDestroyPipelineLayout(context.device->get(), rtPipelineLayout, nullptr);

  vkDestroyDescriptorPool(context.device->get(), rtDescriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(context.device->get(), rtDescriptorSetLayout,
                               nullptr);

  vkDestroyPipeline(context.device->get(), rasterizer->graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(context.device->get(), rasterizer->pipelineLayout, nullptr);
  vkDestroyRenderPass(context.device->get(), rasterizer->renderPass, nullptr);

  vkDestroyDescriptorPool(context.device->get(), rasterizer->descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(context.device->get(), rasterizer->descriptorSetLayout,
                               nullptr);

  for (size_t i = 0; i < context.MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(context.device->get(), renderFinishedSemaphores[i],
                       nullptr);
    vkDestroySemaphore(context.device->get(), imageAvailableSemaphores[i],
                       nullptr);
    vkDestroyFence(context.device->get(), inFlightFences[i], nullptr);
  }
}

void SwapChain::create() {
  PhysicalDevice::SwapChainSupportDetails swapChainSupport =
      context.physicalDevice->querySwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = context.surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  PhysicalDevice::QueueFamilyIndices indices =
      context.physicalDevice->findQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(context.device->get(), &createInfo, nullptr,
                           &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(context.device->get(), swapChain, &imageCount,
                          nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(context.device->get(), swapChain, &imageCount,
                          swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void SwapChain::cleanup() {
  vkDestroyImageView(context.device->get(), rtOutputImageView, nullptr);

  vkDestroyImageView(context.device->get(), rasterizer->depthImageView, nullptr);
  vkFreeMemory(context.device->get(), rasterizer->depthImageMemory, nullptr);

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(context.device->get(), framebuffer, nullptr);
  }

  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(context.device->get(), imageView, nullptr);
  }

  vkDestroySwapchainKHR(context.device->get(), swapChain, nullptr);
}

void SwapChain::recreate() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(context.window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(context.window, &width, &height);
    glfwWaitEvents();
  }

  context.device->waitIdle();

  cleanup();

  create();
  createImageViews();
  rasterizer->refresh();
  createFramebuffers();
}

void SwapChain::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());

  for (uint32_t i = 0; i < swapChainImages.size(); i++) {
    swapChainImageViews[i] = context.device->createImageView(
        swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void SwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    std::array<VkImageView, 2> attachments = {swapChainImageViews[i],
                                              rasterizer->depthImageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = rasterizer->renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(context.device->get(), &framebufferInfo, nullptr,
                            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void SwapChain::copyToBackImage(Image &image) {
  VkCommandBuffer commandBuffer = commandBuffers[currentFrame];
  VkImage backImage = swapChainImages[currentFrame];
  image.transitionLayout(VK_IMAGE_LAYOUT_GENERAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer,
                         true);
  context.device->transitionImageLayout(backImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        commandBuffer, true);
  context.device->copyImage(image.get(), backImage, swapChainExtent,
                            commandBuffer);
  image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_IMAGE_LAYOUT_GENERAL, true);
  context.device->transitionImageLayout(
      backImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer, true);
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  if (vsync) {
    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }
  } else {
    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        return availablePresentMode;
      }
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(context.window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void SwapChain::createCommandBuffers() {
  commandBuffers.resize(context.MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = context.device->getCommandPool();
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  if (vkAllocateCommandBuffers(context.device->get(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void SwapChain::drawFrame() {
  vkWaitForFences(context.device->get(), 1, &inFlightFences[currentFrame],
                  VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      context.device->get(), swapChain, UINT64_MAX,
      imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreate();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  RTUniformBufferObject rtUbo;
  rtUbo.view = context.camera.GetViewMatrix();
  rtUbo.proj = context.camera.GetProjectionMatrix(static_cast<float>(getWidth()),
                                                  static_cast<float>(getHeight()));
  memcpy(rtUniformBufferMapped, &rtUbo, sizeof(rtUbo));

  engine->app.update(rasterizer->uniformBuffersMapped[currentFrame], getWidth(), getHeight());

  vkResetFences(context.device->get(), 1, &inFlightFences[currentFrame]);

  vkResetCommandBuffer(commandBuffers[currentFrame],
                       /*VkCommandBufferResetFlagBits*/ 0);
  if (useRaytracer) {
    raytrace(commandBuffers[currentFrame]);
  } else {
    rasterizer->recordCommandBuffer(commandBuffers[currentFrame], imageIndex, currentFrame);
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(context.device->getGraphicsQueue(), 1, &submitInfo,
                    inFlightFences[currentFrame]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(context.device->getPresentQueue(), &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      engine->context.framebufferResized) {
    engine->context.framebufferResized = false;
    recreate();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  vkQueueWaitIdle(context.device->getPresentQueue());

  currentFrame = (currentFrame + 1) % context.MAX_FRAMES_IN_FLIGHT;
}

void SwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(context.MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(context.MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(context.MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < context.MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(context.device->get(), &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(context.device->get(), &semaphoreInfo, nullptr,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(context.device->get(), &fenceInfo, nullptr,
                      &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

void SwapChain::createRTDescriptorSetLayout() {
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
                                  &rtDescriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create RT descriptor set layout!");
  }
}

void SwapChain::createRTDescriptorPool() {
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
                             &rtDescriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create RT descriptor pool!");
  }
}

void SwapChain::createRTDescriptorSet() {
  rtOutputImage = std::make_unique<Image>(context, swapChainExtent.width,
                                          swapChainExtent.height,
                                          Image::Type::RTOutputImage);
  rtOutputImage->transitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_GENERAL, true);
  rtOutputImageView = rtOutputImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = rtDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &rtDescriptorSetLayout;

  if (vkAllocateDescriptorSets(context.device->get(), &allocInfo,
                               &rtDescriptorSet) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate RT descriptor sets!");
  }

  VkAccelerationStructureKHR tlas = engine->rtx_model->TLAS->accel;

  VkWriteDescriptorSetAccelerationStructureKHR descASInfo{};
  descASInfo.sType =
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
  descASInfo.accelerationStructureCount = 1;
  descASInfo.pAccelerationStructures = &tlas;

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageView = rtOutputImageView;
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
  uniformBufferInfo.buffer = rtUniformBuffer->get();
  uniformBufferInfo.range = sizeof(RTUniformBufferObject);

  std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = rtDescriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType =
      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pNext = &descASInfo;

  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = rtDescriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].dstArrayElement = 0;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].pImageInfo = &imageInfo;

  descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[2].dstSet = rtDescriptorSet;
  descriptorWrites[2].dstBinding = 2;
  descriptorWrites[2].dstArrayElement = 0;
  descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[2].descriptorCount = 1;
  descriptorWrites[2].pBufferInfo = &vertexBufferInfo;

  descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[3].dstSet = rtDescriptorSet;
  descriptorWrites[3].dstBinding = 3;
  descriptorWrites[3].dstArrayElement = 0;
  descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[3].descriptorCount = 1;
  descriptorWrites[3].pBufferInfo = &indexBufferInfo;

  descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[4].dstSet = rtDescriptorSet;
  descriptorWrites[4].dstBinding = 4;
  descriptorWrites[4].dstArrayElement = 0;
  descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[4].descriptorCount = 1;
  descriptorWrites[4].pBufferInfo = &faceBufferInfo;

  descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[5].dstSet = rtDescriptorSet;
  descriptorWrites[5].dstBinding = 5;
  descriptorWrites[5].dstArrayElement = 0;
  descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[5].descriptorCount = 1;
  descriptorWrites[5].pBufferInfo = &uniformBufferInfo;

  vkUpdateDescriptorSets(context.device->get(),
                         static_cast<uint32_t>(descriptorWrites.size()),
                         descriptorWrites.data(), 0, nullptr);
}

void SwapChain::createRTPipeline() {
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
  rtShaderGroups.push_back(group);
  // Miss
  group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  group.generalShader = eMiss;
  rtShaderGroups.push_back(group);
  // Closest Hit
  group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
  group.generalShader = VK_SHADER_UNUSED_KHR;
  group.closestHitShader = eClosestHit;
  rtShaderGroups.push_back(group);

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
  pipelineLayoutCreateInfo.pSetLayouts = &rtDescriptorSetLayout;
  pipelineLayoutCreateInfo.pushConstantRangeCount = pushRanges.size();
  pipelineLayoutCreateInfo.pPushConstantRanges = pushRanges.data();

  vkCreatePipelineLayout(context.device->get(), &pipelineLayoutCreateInfo,
                         nullptr, &rtPipelineLayout);

  VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{};
  rayPipelineInfo.sType =
      VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
  rayPipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
  rayPipelineInfo.pStages = stages.data();
  rayPipelineInfo.groupCount = static_cast<uint32_t>(rtShaderGroups.size());
  rayPipelineInfo.pGroups = rtShaderGroups.data();
  rayPipelineInfo.maxPipelineRayRecursionDepth = 2;
  rayPipelineInfo.layout = rtPipelineLayout;

  vkCreateRayTracingPipelinesKHR(context.device->get(), {}, {}, 1,
                                 &rayPipelineInfo, nullptr, &rtPipeline);
}

void SwapChain::createRTShaderBindingTable() {
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
  uint32_t groupCount = static_cast<uint32_t>(rtShaderGroups.size());
  uint32_t sbtSize = groupCount * handleSizeAligned;

  std::vector<uint8_t> handleStorage(sbtSize);
  if (vkGetRayTracingShaderGroupHandlesKHR(
          context.device->get(), rtPipeline, 0, groupCount, sbtSize,
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

void SwapChain::raytrace(const VkCommandBuffer &commandBuffer) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                    rtPipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                          rtPipelineLayout, 0, 1, &rtDescriptorSet, 0, nullptr);
  vkCmdPushConstants(commandBuffer, rtPipelineLayout,
                     VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(int), &frame);
  if (context.camera.getHasMoved()) {
    standingFrames = 0;
  }
  vkCmdPushConstants(commandBuffer, rtPipelineLayout,
                     VK_SHADER_STAGE_RAYGEN_BIT_KHR, sizeof(int), sizeof(int),
                     &standingFrames);
  vkCmdTraceRaysKHR(commandBuffer, &raygenRegion, &missRegion, &hitRegion,
                    &callRegion, swapChainExtent.width, swapChainExtent.height,
                    2);
  copyToBackImage(*rtOutputImage);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
  context.device->waitIdle();
  frame++;
  standingFrames++;
}

void SwapChain::createRTUniformBuffer() {
  VkDeviceSize bufferSize = sizeof(RTUniformBufferObject);

  rtUniformBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Uniform, bufferSize);

  vkMapMemory(context.device->get(), rtUniformBuffer->getDeviceMemory(), 0,
              bufferSize, 0, &rtUniformBufferMapped);
}

void SwapChain::refresh() {
  rasterizer->createDescriptorSets();
}
