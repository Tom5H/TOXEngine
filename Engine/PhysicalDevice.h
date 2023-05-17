#ifndef TOXENGINE_PHYSICALDEVICE_H_
#define TOXENGINE_PHYSICALDEVICE_H_

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

class Context;
struct QueueFamilyIndices;
struct SwapChainSupportDetails;

class PhysicalDevice {
public:
  PhysicalDevice(Context &context);
  ~PhysicalDevice() {}

  VkPhysicalDevice get() { return physicalDevice; }
  bool checkDeviceExtensionSupport();
  QueueFamilyIndices findQueueFamilies();
  SwapChainSupportDetails querySwapChainSupport();
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);

protected:
  virtual bool hasRequiredFeatures();

private:
  Context &context;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

#endif // TOXENGINE_PHYSICALDEVICE_H_
