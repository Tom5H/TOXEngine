#ifndef TOXENGINE_ENGINE_MODEL_H_
#define TOXENGINE_ENGINE_MODEL_H_

#include "Buffer.h"
#include "Vertex.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Context;

class Model {
public:
  Model(Context &context, const std::string path);

  uint32_t getIndexCount() const { return nbIndices; }
  uint32_t getVertexCount() const { return nbVertices; }

  std::unique_ptr<Buffer> vertexBuffer;
  std::unique_ptr<Buffer> indexBuffer;

  std::vector<uint32_t> indices;
  
private:
  Context &context;

  std::vector<Vertex> vertices;

  void createVertexBuffer();
  void createIndexBuffer();

  uint32_t nbIndices;
  uint32_t nbVertices;
};

#endif // TOXENGINE_ENGINE_MODEL_H_
