#ifndef TOXENGINE_ENGINE_SWAPCHAIN_H_
#define TOXENGINE_ENGINE_SWAPCHAIN_H_

#include "Buffer.h"
#include "Image.h"
#include "Rasterizer.h"
#include "Raytracer.h"

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

  void refresh();
  void drawFrame();
  void copyToBackImage(Image &image);

  VkSwapchainKHR get() { return swapChain; }
  VkExtent2D getExtent() { return swapChainExtent; }
  uint32_t getWidth() { return swapChainExtent.width; }
  uint32_t getHeight() { return swapChainExtent.height; }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkFramebuffer getFramebuffer(uint32_t index) {
    return swapChainFramebuffers[index];
  }

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

  Context &context;
  TOXEngine *engine;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::unique_ptr<Rasterizer> rasterizer;

  std::unique_ptr<Raytracer> raytracer;

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  uint32_t currentFrame = 0;
  bool vsync = false;
};

#endif // TOXENGINE_ENGINE_SWAPCHAIN_H_
