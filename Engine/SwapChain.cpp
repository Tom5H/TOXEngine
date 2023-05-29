#include "SwapChain.h"

#include "Buffer.h"
#include "Image.h"
#include "Rasterizer.h"
#include "Raytracer.h"
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

  rasterizer = std::make_unique<Rasterizer>(context, engine, this);

  createFramebuffers();
  createSyncObjects();
  createCommandBuffers();

  raytracer = std::make_unique<Raytracer>(context, engine, this);
}

SwapChain::~SwapChain() {
  cleanup();

  vkDestroyPipeline(context.device->get(), raytracer->pipeline, nullptr);
  vkDestroyPipelineLayout(context.device->get(), raytracer->pipelineLayout,
                          nullptr);

  vkDestroyDescriptorPool(context.device->get(), raytracer->descriptorPool,
                          nullptr);

  vkDestroyDescriptorSetLayout(context.device->get(),
                               raytracer->descriptorSetLayout, nullptr);

  vkDestroyPipeline(context.device->get(), rasterizer->graphicsPipeline,
                    nullptr);
  vkDestroyPipelineLayout(context.device->get(), rasterizer->pipelineLayout,
                          nullptr);
  vkDestroyRenderPass(context.device->get(), rasterizer->renderPass, nullptr);

  vkDestroyDescriptorPool(context.device->get(), rasterizer->descriptorPool,
                          nullptr);

  vkDestroyDescriptorSetLayout(context.device->get(),
                               rasterizer->descriptorSetLayout, nullptr);

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
  vkDestroyImageView(context.device->get(), raytracer->outputImageView,
                     nullptr);

  vkDestroyImageView(context.device->get(), rasterizer->depthImageView,
                     nullptr);
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
  rtUbo.proj = context.camera.GetProjectionMatrix(
      static_cast<float>(getWidth()), static_cast<float>(getHeight()));
  memcpy(raytracer->uniformBufferMapped, &rtUbo, sizeof(rtUbo));

  engine->app.update(rasterizer->uniformBuffersMapped[currentFrame], getWidth(),
                     getHeight());

  vkResetFences(context.device->get(), 1, &inFlightFences[currentFrame]);

  vkResetCommandBuffer(commandBuffers[currentFrame],
                       /*VkCommandBufferResetFlagBits*/ 0);
  if (useRaytracer) {
    raytracer->recordCommandBuffer(commandBuffers[currentFrame]);
  } else {
    rasterizer->recordCommandBuffer(commandBuffers[currentFrame], imageIndex,
                                    currentFrame);
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

void SwapChain::refresh() {
  rasterizer->createDescriptorSets();
  raytracer->createDescriptorSet();
}
