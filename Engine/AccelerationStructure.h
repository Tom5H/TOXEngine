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
  ~AccelerationStructure();

  std::unique_ptr<Buffer> buffer;

  VkAccelerationStructureKHR accel;
  VkWriteDescriptorSetAccelerationStructureKHR accelInfo;

private:
  Context &context;
  std::unique_ptr<Buffer> scratch;
};

#endif // TOXENGINE_ACCELERATIONSTRUCTURE_H_
