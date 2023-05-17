#ifndef TOXENGINE_H_
#define TOXENGINE_H_

#include "../App/App.h"
#include "Buffer.h"
#include "Context.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "SwapChain.h"
#include "Vertex.h"
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../resources/models/viking_room.obj";
const std::string TEXTURE_PATH = "../resources/textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

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

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class TOXEngine {
public:
  TOXEngine() : app(this) {}
  ~TOXEngine() { cleanup(); }

  App app;
  Context context;

  void run() {
    initVulkan();
    mainLoop();
  }

  std::shared_ptr<PhysicalDevice> getPhysicalDevice() { return physicalDevice; }
  std::shared_ptr<Device> getDevice() { return device; }

private:
  std::shared_ptr<PhysicalDevice> physicalDevice;
  std::shared_ptr<Device> device;

  std::shared_ptr<SwapChain> swapChain;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;

public:
  VkImageView textureImageView;
  VkSampler textureSampler;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Buffer> indexBuffer;
private:
  void initVulkan() {
    physicalDevice = std::make_shared<PhysicalDevice>(context);
    device = std::make_shared<Device>(physicalDevice);
    swapChain = std::make_shared<SwapChain>(this);
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    swapChain->createDescriptorSets();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(context.window)) {
      glfwPollEvents();
      swapChain->drawFrame();
    }

    device->waitIdle();
  }

  void cleanup() {
    // cleanupSwapChain();

    // vkDestroyPipeline(device->get(), graphicsPipeline, nullptr);
    // vkDestroyPipelineLayout(device->get(), pipelineLayout, nullptr);
    // vkDestroyRenderPass(device->get(), renderPass, nullptr);

    /*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(device->get(), uniformBuffers[i], nullptr);
      vkFreeMemory(device->get(), uniformBuffersMemory[i], nullptr);
      }*/

    // vkDestroyDescriptorPool(device->get(), descriptorPool, nullptr);

    vkDestroySampler(device->get(), textureSampler, nullptr);
    vkDestroyImageView(device->get(), textureImageView, nullptr);

    vkDestroyImage(device->get(), textureImage, nullptr);
    vkFreeMemory(device->get(), textureImageMemory, nullptr);

    // vkDestroyDescriptorSetLayout(device->get(), descriptorSetLayout,
    // nullptr);

    // vkDestroyBuffer(device->get(), indexBuffer, nullptr);
    // vkFreeMemory(device->get(), indexBufferMemory, nullptr);

    // vkDestroyBuffer(device->get(), vertexBuffer, nullptr);
    // vkFreeMemory(device->get(), vertexBufferMemory, nullptr);

    /*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device->get(), renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device->get(), imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(device->get(), inFlightFences[i], nullptr);
      }*/

    // vkDestroyCommandPool(device->get(), commandPool, nullptr);

    // vkDestroyDevice(device, nullptr)

    // if (enableValidationLayers) {
    //   DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    // }

    // vkDestroySurfaceKHR(instance, surface, nullptr);
    // vkDestroyInstance(instance, nullptr);

    // glfwDestroyWindow(window);

    // glfwTerminate();
  }

  bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
  }

  void createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
      throw std::runtime_error("failed to load texture image!");
    }

    Buffer stagingBuffer(this, Buffer::Type::Staging, imageSize);
    // VkBuffer stagingBuffer;
    // VkDeviceMemory stagingBufferMemory;
    // createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    //                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //              stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device->get(), stagingBuffer.getDeviceMemory(), 0, imageSize, 0,
                &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device->get(), stagingBuffer.getDeviceMemory());

    stbi_image_free(pixels);

    createImage(
        texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer.get(), textureImage,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // vkDestroyBuffer(device->get(), stagingBuffer, nullptr);
    // vkFreeMemory(device->get(), stagingBufferMemory, nullptr);
  }

  void createTextureImageView() {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_ASPECT_COLOR_BIT);
  }

  void createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice->get(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device->get(), &samplerInfo, nullptr,
                        &textureSampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
  }

public:
  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device->get(), &viewInfo, nullptr, &imageView) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
  }

  void createImage(uint32_t width, uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device->get(), &imageInfo, nullptr, &image) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->get(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = physicalDevice->findMemoryType(
        memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->get(), &allocInfo, nullptr, &imageMemory) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device->get(), image, imageMemory, 0);
  }

private:
  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
      throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    device->endSingleTimeCommands(commandBuffer);
  }

  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height) {
    VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    device->endSingleTimeCommands(commandBuffer);
  }

  void loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          MODEL_PATH.c_str())) {
      throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes) {
      for (const auto &index : shape.mesh.indices) {
        Vertex vertex{};

        vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                      attrib.vertices[3 * index.vertex_index + 1],
                      attrib.vertices[3 * index.vertex_index + 2]};

        vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                           1.0f -
                               attrib.texcoords[2 * index.texcoord_index + 1]};

        vertex.color = {1.0f, 1.0f, 1.0f};

        if (uniqueVertices.count(vertex) == 0) {
          uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
          vertices.push_back(vertex);
        }

        indices.push_back(uniqueVertices[vertex]);
      }
    }
  }

  void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer(this, Buffer::Type::Staging, bufferSize);
    // VkBuffer stagingBuffer;
    // VkDeviceMemory stagingBufferMemory;
    // createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    //                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //              stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device->get(), stagingBuffer.getDeviceMemory(), 0, bufferSize,
                0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device->get(), stagingBuffer.getDeviceMemory());

    vertexBuffer =
        std::make_shared<Buffer>(this, Buffer::Type::Vertex, bufferSize);
    // createBuffer(
    //     bufferSize,
    //     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer,
    //     vertexBufferMemory);

    vertexBuffer->copy(stagingBuffer, bufferSize);
    // copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // vkDestroyBuffer(device->get(), stagingBuffer, nullptr);
    // vkFreeMemory(device->get(), stagingBufferMemory, nullptr);
  }

  void createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingBuffer(this, Buffer::Type::Staging, bufferSize);
    // VkBuffer stagingBuffer;
    // VkDeviceMemory stagingBufferMemory;
    // createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    //                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //              stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device->get(), stagingBuffer.getDeviceMemory(), 0, bufferSize,
                0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device->get(), stagingBuffer.getDeviceMemory());

    indexBuffer =
        std::make_shared<Buffer>(this, Buffer::Type::Index, bufferSize);
    // createBuffer(
    //     bufferSize,
    //     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    indexBuffer->copy(stagingBuffer, bufferSize);
    // copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    // vkDestroyBuffer(device->get(), stagingBuffer, nullptr);
    // vkFreeMemory(device->get(), stagingBufferMemory, nullptr);
  }

public:
  VkShaderModule createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->get(), &createInfo, nullptr,
                             &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

private:
};

#endif // TOXENGINE_H_
