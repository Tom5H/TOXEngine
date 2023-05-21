#ifndef TOXENGINE_RTXMODEL_H_
#define TOXENGINE_RTXMODEL_H_

#include "AccelerationStructure.h"
#include "Model.h"

#include <memory>

class RTXModel : public Model {
public:
  RTXModel(Context &context, const std::string path);

  std::shared_ptr<AccelerationStructure> BLAS;
  std::shared_ptr<AccelerationStructure> TLAS;

private:
  std::shared_ptr<Buffer> instancesBuffer;
};

#endif // TOXENGINE_RTXMODEL_H_
