#ifndef TOXENGINE_ACCELERATIONSTRUCTURE_H_
#define TOXENGINE_ACCELERATIONSTRUCTURE_H_

#include "Buffer.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>

class Context;

class AccelerationStructure {
public:
  AccelerationStructure(Context &context,
                        VkAccelerationStructureGeometryKHR geometry,
                        uint32_t primitiveCount,
                        VkAccelerationStructureTypeKHR type);

  std::shared_ptr<Buffer> buffer;

private:
  Context &context;
  VkAccelerationStructureKHR accel;
  VkWriteDescriptorSetAccelerationStructureKHR accelInfo;
};

#endif // TOXENGINE_ACCELERATIONSTRUCTURE_H_
