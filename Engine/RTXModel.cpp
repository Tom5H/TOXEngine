#include "RTXModel.h"

#include "AccelerationStructure.h"
#include "Buffer.h"
#include "Context.h"
#include "Vertex.h"

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vulkan/vulkan_core.h>

RTXModel::RTXModel(Context &context, const std::string path)
    : Model(context, path) {
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

  BLAS = std::make_shared<AccelerationStructure>(
      context, asGeom, primitiveCount,
      VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

  // TLAS
  // TODO to support mulitple models move TLAS out of here
  VkTransformMatrixKHR transformMatrix{1.0f, 0.0f, 0.0f, 0.0f, ///////
                                       0.0f, 1.0f, 0.0f, 0.0f, ///////
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

  instancesBuffer =
      std::make_shared<Buffer>(context, Buffer::Type::AccelInput,
                               sizeof(VkAccelerationStructureInstanceKHR));

  void *mapped;
  vkMapMemory(context.device->get(), instancesBuffer->getDeviceMemory(), 0,
              sizeof(VkAccelerationStructureInstanceKHR), 0, &mapped);
  memcpy(mapped, &asInstance, sizeof(VkAccelerationStructureInstanceKHR));

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

  TLAS = std::make_shared<AccelerationStructure>(
      context, instanceGeometry, 1,
      VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
}
