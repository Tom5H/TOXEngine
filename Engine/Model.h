#ifndef TOXENGINE_MODEL_H_
#define TOXENGINE_MODEL_H_

#include "Buffer.h"
#include "SwapChain.h"
#include "Vertex.h"

#include <memory>
#include <string>
#include <vector>

class TOXEngine;

class Model {
  friend class SwapChain;
public:
  Model(TOXEngine *engine, const std::string path);

private:
  void createVertexBuffer(std::vector<Vertex> &vertices);
  void createIndexBuffer();
  
  TOXEngine *engine;
  std::vector<uint32_t> indices;
  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Buffer> indexBuffer;
};

#endif // TOXENGINE_MODEL_H_
