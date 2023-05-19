#ifndef TOXENGINE_PHYSICALDEVICE_H_
#define TOXENGINE_PHYSICALDEVICE_H_

#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <vector>

class Context;
struct QueueFamilyIndices;
struct SwapChainSupportDetails;

class PhysicalDevice {
public:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  PhysicalDevice(Context *context);
  ~PhysicalDevice() {}

  VkPhysicalDevice get() { return physicalDevice; }
  bool checkDeviceExtensionSupport();
  QueueFamilyIndices findQueueFamilies();
  SwapChainSupportDetails querySwapChainSupport();
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);

protected:
  virtual bool hasRequiredFeatures();

private:
  Context *context;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

#endif // TOXENGINE_PHYSICALDEVICE_H_
