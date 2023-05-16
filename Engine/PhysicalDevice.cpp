#include "PhysicalDevice.h"
#include "TOXEngine.h"

#include <stdexcept>
#include <vector>

PhysicalDevice::PhysicalDevice(TOXEngine *engine) : engine(engine) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(engine->getVkInstance(), &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(engine->getVkInstance(), &deviceCount,
                             devices.data());

  for (const auto &device : devices) {
    physicalDevice = device;
    if (hasRequiredFeatures()) {
      break;
    }
    physicalDevice = VK_NULL_HANDLE;
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

bool PhysicalDevice::checkDeviceExtensionSupport() {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices PhysicalDevice::findQueueFamilies() {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i,
                                         engine->getSurface(), &presentSupport);

    if (presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

SwapChainSupportDetails PhysicalDevice::querySwapChainSupport() {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, engine->getSurface(), &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, engine->getSurface(),
                                       &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, engine->getSurface(),
                                         &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, engine->getSurface(), &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, engine->getSurface(), &presentModeCount,
        details.presentModes.data());
  }

  return details;
}

VkFormat PhysicalDevice::findSupportedFormat(const std::vector<VkFormat> &candidates,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

bool PhysicalDevice::hasRequiredFeatures() {
  QueueFamilyIndices indices = findQueueFamilies();

  bool extensionsSupported = checkDeviceExtensionSupport();

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}
