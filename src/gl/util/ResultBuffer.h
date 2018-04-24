#ifndef RESULTBUFFER_H
#define RESULTBUFFER_H

#include "Buffer.h"

#include "gl/textures/Texture2D.h"
#include "pathtracer/pathtracer.h"

class ResultBuffer : public Buffer {
public:
  ResultBuffer(int width, int height);

  const CS123::GL::Texture2D &color_texture() const;

private:
  CS123::GL::Texture2D m_color;
};

#endif // RESULTBUFFER_H
