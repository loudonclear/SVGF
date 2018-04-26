#ifndef SVGFGBUFFER_H
#define SVGFGBUFFER_H

#include "Buffer.h"

#include "gl/textures/Texture2D.h"

class SVGFGBuffer : public Buffer {
public:
  SVGFGBuffer(int width, int height);

  const CS123::GL::Texture2D &position_mesh_id_texture() const;
  const CS123::GL::Texture2D &normal_texture() const;

private:
  CS123::GL::Texture2D gPositionMeshID, gNormal;
  unsigned int rboDepth;
};

#endif // SVGFGBUFFER_H
