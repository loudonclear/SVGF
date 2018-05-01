#ifndef SVGFGBUFFER_H
#define SVGFGBUFFER_H

#include "Buffer.h"

#include "util/util.h"
#include "gl/textures/Texture2D.h"

class SVGFGBuffer : public Buffer {
public:
  SVGFGBuffer(int width, int height);

  const CS123::GL::Texture2D &position_mesh_id_texture() const;
  const CS123::GL::Texture2D &normal_texture() const;

  // set data for position/id and normal textures.
  void set_textures(const RenderBuffers& buffers);

private:
  CS123::GL::Texture2D gPositionMeshID, gNormal;
  unsigned int rboDepth;
};

#endif // SVGFGBUFFER_H
