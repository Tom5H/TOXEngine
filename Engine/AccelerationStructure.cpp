#include "AccelerationStructure.h"

#include "Buffer.h"
#include "Context.h"
#include <memory>

AccelerationStructure::AccelerationStructure(
    Context &context, VkAccelerationStructureGeometryKHR geometry,
    uint32_t primitiveCount, VkAccelerationStructureTypeKHR type)
    : context(context) {
  VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
  buildGeometryInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  buildGeometryInfo.type = type;
  buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
  buildGeometryInfo.flags =
      VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
  buildGeometryInfo.geometryCount = 1;
  buildGeometryInfo.pGeometries = &geometry;

  VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo;
  buildSizesInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  vkGetAccelerationStructureBuildSizesKHR(
      context.device->get(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
      &buildGeometryInfo, &primitiveCount, &buildSizesInfo);

  buffer = std::make_unique<Buffer>(context, Buffer::Type::AccelStorage,
                                    buildSizesInfo.accelerationStructureSize);

  VkAccelerationStructureCreateInfoKHR asCreateInfo{};
  asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
  asCreateInfo.buffer = buffer->get();
  asCreateInfo.size = buildSizesInfo.accelerationStructureSize;
  asCreateInfo.type = type;

  vkCreateAccelerationStructureKHR(context.device->get(), &asCreateInfo,
                                   nullptr, &accel);

  scratch = std::make_unique<Buffer>(context, Buffer::Type::Scratch,
                                     buildSizesInfo.buildScratchSize);

  buildGeometryInfo.scratchData.deviceAddress = scratch->getDeviceAddress();
  buildGeometryInfo.dstAccelerationStructure = accel;

  VkAccelerationStructureBuildRangeInfoKHR offset{};
  offset.firstVertex = 0;
  offset.primitiveCount = primitiveCount;
  offset.primitiveOffset = 0;
  offset.transformOffset = 0;

  VkAccelerationStructureBuildRangeInfoKHR *p_offset = &offset;

  VkCommandBuffer commandBuffer = context.device->beginSingleTimeCommands();

  vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo,
                                      &p_offset);

  context.device->endSingleTimeCommands(commandBuffer);

  accelInfo.sType =
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
  accelInfo.accelerationStructureCount = 1;
  accelInfo.pAccelerationStructures = &accel;
}

AccelerationStructure::~AccelerationStructure() {
  vkDestroyAccelerationStructureKHR(context.device->get(), accel, nullptr);
}
