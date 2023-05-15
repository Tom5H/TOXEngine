#ifndef TOXENGINE_PHYSICALDEVICE_H_
#define TOXENGINE_PHYSICALDEVICE_H_

#include <vulkan/vulkan.h>

class TOXEngine;
struct QueueFamilyIndices;
struct SwapChainSupportDetails;

class PhysicalDevice {
public:
  PhysicalDevice(TOXEngine *engine);
  ~PhysicalDevice() {}

  VkPhysicalDevice get() { return physicalDevice; }
  bool checkDeviceExtensionSupport();
  QueueFamilyIndices findQueueFamilies();
  SwapChainSupportDetails querySwapChainSupport();

protected:
  virtual bool hasRequiredFeatures();

private:
  TOXEngine *engine;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

#endif // TOXENGINE_PHYSICALDEVICE_H_