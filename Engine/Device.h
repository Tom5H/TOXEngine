#ifndef TOXENGINE_DEVICE_H_
#define TOXENGINE_DEVICE_H_

#include "PhysicalDevice.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vulkan/vulkan_core.h>

class TOXEngine;

class Device {
public:
  Device(std::shared_ptr<PhysicalDevice> physicalDevice);
  ~Device();

  VkDevice get() { return device; }
  VkQueue getGraphicsQueue() { return graphicsQueue; }
  VkQueue getPresentQueue() { return presentQueue; }
  VkCommandPool getCommandPool() { return commandPool; }
  void waitIdle();
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
  void create();
  void createCommandPool();

  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkCommandPool commandPool;
  std::shared_ptr<PhysicalDevice> physicalDevice;
};

#endif // TOXENGINE_DEVICE_H_
