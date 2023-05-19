#ifndef TOXENGINE_MODEL_H_
#define TOXENGINE_MODEL_H_

#include "Buffer.h"
#include "Vertex.h"

#include <memory>
#include <string>
#include <vector>

class Context;

class Model {
  friend class SwapChain;
public:
  Model(Context &context, const std::string path);

private:
  void createVertexBuffer(std::vector<Vertex> &vertices);
  void createIndexBuffer();
  
  Context &context;
  std::vector<uint32_t> indices;
  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Buffer> indexBuffer;
};

#endif // TOXENGINE_MODEL_H_
