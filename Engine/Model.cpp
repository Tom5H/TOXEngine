#include "Model.h"

#include "TOXEngine.h"
#include "Vertex.h"

#include <cstdint>
#include <tiny_obj_loader.h>
#include <vector>

Model::Model(TOXEngine *engine, const std::string path)
  : engine(engine)
{
  std::vector<Vertex> vertices;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        path.c_str())) {
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
                         1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }

  createVertexBuffer(vertices);
  createIndexBuffer();
}

void Model::createVertexBuffer(std::vector<Vertex> &vertices) {
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  Buffer stagingBuffer(engine, Buffer::Type::Staging, bufferSize);
  // VkBuffer stagingBuffer;
  // VkDeviceMemory stagingBufferMemory;
  // createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
  //                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //              stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(engine->getDevice()->get(), stagingBuffer.getDeviceMemory(), 0, bufferSize, 0,
              &data);
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(engine->getDevice()->get(), stagingBuffer.getDeviceMemory());

  vertexBuffer =
      std::make_shared<Buffer>(engine, Buffer::Type::Vertex, bufferSize);
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

void Model::createIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  Buffer stagingBuffer(engine, Buffer::Type::Staging, bufferSize);
  // VkBuffer stagingBuffer;
  // VkDeviceMemory stagingBufferMemory;
  // createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
  //                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //              stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(engine->getDevice()->get(), stagingBuffer.getDeviceMemory(), 0, bufferSize, 0,
              &data);
  memcpy(data, indices.data(), (size_t)bufferSize);
  vkUnmapMemory(engine->getDevice()->get(), stagingBuffer.getDeviceMemory());

  indexBuffer = std::make_shared<Buffer>(engine, Buffer::Type::Index, bufferSize);
  // createBuffer(
  //     bufferSize,
  //     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

  indexBuffer->copy(stagingBuffer, bufferSize);
  // copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  // vkDestroyBuffer(device->get(), stagingBuffer, nullptr);
  // vkFreeMemory(device->get(), stagingBufferMemory, nullptr);
}
