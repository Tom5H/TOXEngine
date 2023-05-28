#include "RTXModel.h"

#include "AccelerationStructure.h"
#include "Buffer.h"

#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>

RTXModel::RTXModel(Context &context, const std::string path)
    : context(context) {
  load(path);
  VkDeviceAddress vertexAddress = vertexBuffer->getDeviceAddress();
  VkDeviceAddress indexAddress = indexBuffer->getDeviceAddress();

  uint32_t primitiveCount = getIndexCount() / 3;

  VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
  triangles.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
  triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
  triangles.vertexData.deviceAddress = vertexAddress;
  triangles.vertexStride = sizeof(Vertex);
  triangles.indexType = VK_INDEX_TYPE_UINT32;
  triangles.indexData.deviceAddress = indexAddress;
  triangles.maxVertex = getVertexCount();

  VkAccelerationStructureGeometryKHR asGeom{};
  asGeom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  asGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
  asGeom.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
  asGeom.geometry.triangles = triangles;

  BLAS = std::make_unique<AccelerationStructure>(
      context, asGeom, primitiveCount,
      VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

  // TLAS
  // TODO to support mulitple models move TLAS out of here
  // VkTransformMatrixKHR transformMatrix{0.05f, 0.0f,   0.0f,  0.0f, ///////
  // 0.0f,  0.0f,   0.05f, 0.0f, ///////
  //                                   0.0f,  -0.05f, 0.0f,  0.0f};

  VkTransformMatrixKHR transformMatrix{1.0f, 0.0f, 0.0f, 0.0f, //
                                       0.0f, 1.0f, 0.0f, 0.0f, //
                                       0.0f, 0.0f, 1.0f, 0.0f};

  VkAccelerationStructureInstanceKHR asInstance{};
  // TODO this should be the model transform (model matrix)
  asInstance.transform = transformMatrix;
  asInstance.instanceCustomIndex =
      0; // todo unique per model - shader access index
  asInstance.accelerationStructureReference = BLAS->buffer->getDeviceAddress();
  asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
  asInstance.mask = 0xFF;
  asInstance.instanceShaderBindingTableRecordOffset = 0; // (hit group)

  instancesBuffer = std::make_unique<Buffer>(
      context, Buffer::Type::AccelInput,
      sizeof(VkAccelerationStructureInstanceKHR), &asInstance);

  VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
  instancesData.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
  instancesData.arrayOfPointers = false;
  instancesData.data.deviceAddress = instancesBuffer->getDeviceAddress();

  VkAccelerationStructureGeometryKHR instanceGeometry{};
  instanceGeometry.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  instanceGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
  instanceGeometry.geometry.instances = instancesData;
  instanceGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

  TLAS = std::make_unique<AccelerationStructure>(
      context, instanceGeometry, 1,
      VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
}

void RTXModel::load(const std::string path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(),
                        "../resources/models")) {
    throw std::runtime_error(warn + err);
  }

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};
      vertex.pos[0] = attrib.vertices[3 * index.vertex_index + 0];
      vertex.pos[1] = -attrib.vertices[3 * index.vertex_index + 1];
      vertex.pos[2] = attrib.vertices[3 * index.vertex_index + 2];
      vertices.push_back(vertex);
      indices.push_back(static_cast<uint32_t>(indices.size()));
    }
    for (const auto &matIndex : shape.mesh.material_ids) {
      Face face;
      face.diffuse[0] = materials[matIndex].diffuse[0];
      face.diffuse[1] = materials[matIndex].diffuse[1];
      face.diffuse[2] = materials[matIndex].diffuse[2];
      face.emission[0] = materials[matIndex].emission[0];
      face.emission[1] = materials[matIndex].emission[1];
      face.emission[2] = materials[matIndex].emission[2];
      faces.push_back(face);
    }
  }

  nbIndices = indices.size();
  nbVertices = vertices.size();
  nbFaces = faces.size();

  createVertexBuffer();
  createIndexBuffer();
  createFaceBuffer();
}

void RTXModel::createVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(vertices[0]) * nbVertices;
  Buffer stagingBuffer(context, Buffer::Type::Staging, bufferSize,
                       vertices.data());
  vertexBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Vertex, bufferSize);
  vertexBuffer->copy(stagingBuffer, bufferSize);
  stagingBuffer.cleanup();
}

void RTXModel::createIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(indices[0]) * nbIndices;
  Buffer stagingBuffer(context, Buffer::Type::Staging, bufferSize,
                       indices.data());
  indexBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Index, bufferSize);
  indexBuffer->copy(stagingBuffer, bufferSize);
  stagingBuffer.cleanup();
}

void RTXModel::createFaceBuffer() {
  VkDeviceSize bufferSize = sizeof(faces[0]) * nbFaces;
  Buffer stagingBuffer(context, Buffer::Type::Staging, bufferSize,
                       faces.data());
  faceBuffer =
      std::make_unique<Buffer>(context, Buffer::Type::Face, bufferSize);
  faceBuffer->copy(stagingBuffer, bufferSize);
  stagingBuffer.cleanup();
}
