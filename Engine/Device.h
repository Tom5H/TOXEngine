#ifndef TOXENGINE_DEVICE_H_
#define TOXENGINE_DEVICE_H_

#include "PhysicalDevice.h"

#include <vulkan/vulkan.h>

#include <memory>

class Context;
class TOXEngine;

class Device {
public:
  Device(Context *context, std::shared_ptr<PhysicalDevice> physicalDevice);
  ~Device();

  VkDevice get() { return device; }
  VkQueue getGraphicsQueue() { return graphicsQueue; }
  VkQueue getPresentQueue() { return presentQueue; }
  VkCommandPool getCommandPool() { return commandPool; }
  void waitIdle();
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags);

private:
  void create();
  void createCommandPool();

  Context *context;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkCommandPool commandPool;
  std::shared_ptr<PhysicalDevice> physicalDevice;
};

#endif // TOXENGINE_DEVICE_H_
