#include "Model.h"

#include "TOXEngine.h"
#include "Vertex.h"

#include <memory>
#include <tiny_obj_loader.h>

#include <cstring>

Model::Model(Context &context, const std::string path) : context(context) {
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

  nbIndices = indices.size();
  nbVertices = vertices.size();

  createVertexBuffer();
  createIndexBuffer();
}

void Model::createVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(vertices[0]) * nbVertices;

  Buffer stagingBuffer(context, Buffer::Type::Staging, bufferSize,
                       vertices.data());

  vertexBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Vertex, bufferSize);

  vertexBuffer->copy(stagingBuffer, bufferSize);
  stagingBuffer.cleanup();
}

void Model::createIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(indices[0]) * nbIndices;

  Buffer stagingBuffer(context, Buffer::Type::Staging, bufferSize,
                       indices.data());

  indexBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Index, bufferSize);

  indexBuffer->copy(stagingBuffer, bufferSize);
  stagingBuffer.cleanup();
}
