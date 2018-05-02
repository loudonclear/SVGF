#ifndef DISPLAYBUFFER_H
#define DISPLAYBUFFER_H


#include "Buffer.h"

#include "gl/textures/Texture2D.h"
#include "pathtracer/pathtracer.h"

class DisplayBuffer : public Buffer {
public:
  DisplayBuffer(int width, int height);

  const CS123::GL::Texture2D &color_texture() const;

private:
  CS123::GL::Texture2D m_color;
};


#endif // DISPLAYBUFFER_H
