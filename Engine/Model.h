#ifndef TOXENGINE_MODEL_H_
#define TOXENGINE_MODEL_H_

#include "Buffer.h"
#include "Vertex.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Context;

class Model {
  friend class SwapChain;

public:
  Model(Context &context, const std::string path);

  uint32_t getIndexCount() const { return nbIndices; }
  uint32_t getVertexCount() const { return nbVertices; }

private:
  Context &context;

  std::vector<uint32_t> indices;
  std::vector<Vertex> vertices;
  std::unique_ptr<Buffer> vertexBuffer;
  std::unique_ptr<Buffer> indexBuffer;

  void createVertexBuffer();
  void createIndexBuffer();

  uint32_t nbIndices;
  uint32_t nbVertices;
};

#endif // TOXENGINE_MODEL_H_
