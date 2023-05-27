#ifndef TOXENGINE_RTXMODEL_H_
#define TOXENGINE_RTXMODEL_H_

#include "AccelerationStructure.h"
#include "Buffer.h"
#include "Context.h"
#include "Face.h"
#include "Vertex.h"

#include <cstdint>
#include <memory>
#include <vector>

class RTXModel {
  struct Vertex {
    float pos[3];
  };
public:
  RTXModel(Context &context, const std::string path);

  uint32_t getIndexCount() const { return nbIndices; }
  uint32_t getVertexCount() const { return nbVertices; }
  uint32_t getFaceCount() const { return nbFaces; }

  std::vector<uint32_t> indices;
  std::shared_ptr<Buffer> indexBuffer;
  
  std::vector<Vertex> vertices;
  std::shared_ptr<Buffer> vertexBuffer;

  std::vector<Face> faces;
  std::shared_ptr<Buffer> faceBuffer;

  std::shared_ptr<AccelerationStructure> BLAS;
  std::shared_ptr<AccelerationStructure> TLAS;

private:
  void createVertexBuffer();
  void createIndexBuffer();
  void createFaceBuffer();

  Context &context;
  
  uint32_t nbIndices;
  uint32_t nbVertices;
  uint32_t nbFaces;
  
  std::shared_ptr<Buffer> instancesBuffer;

  void load(const std::string path);
};

#endif // TOXENGINE_RTXMODEL_H_
