#ifndef TOXENGINE_CONTEXT_H_
#define TOXENGINE_CONTEXT_H_

#include "Camera.h"
#include "Device.h"
#include "PhysicalDevice.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <vector>

class Context {
public:
  Context();
  ~Context();

  const int MAX_FRAMES_IN_FLIGHT = 3;

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME};

  GLFWwindow *window;
  bool framebufferResized = false;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;
  std::shared_ptr<PhysicalDevice> physicalDevice;
  std::unique_ptr<Device> device;

  Camera camera;

private:
  void initWindow();

  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) {
    auto context =
        reinterpret_cast<Context *>(glfwGetWindowUserPointer(window));
    context->framebufferResized = true;
  }

  static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    auto context =
        reinterpret_cast<Context *>(glfwGetWindowUserPointer(window));
    context->camera.ProcessMouseMovement(xpos, ypos);
  }

  static void scroll_callback(GLFWwindow *window, double xoffset,
                              double yoffset) {
    auto context =
        reinterpret_cast<Context *>(glfwGetWindowUserPointer(window));
    context->camera.ProcessMouseScroll(yoffset);
  }

  static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                    int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  void createInstance();
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void setupDebugMessenger();
  void createSurface();
  bool checkValidationLayerSupport();

  std::vector<const char *> getRequiredExtensions();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugUtilsMessengerEXT *pDebugMessenger);

  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks *pAllocator);
};

#endif
