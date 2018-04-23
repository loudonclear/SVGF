#ifndef COLORHISTORYBUFFER_H
#define COLORHISTORYBUFFER_H

#include "gl/textures/Texture2D.h"

// Reprents a GL FBO for storing color and variance history. Data is
// stored in RGBA format, where RGB represents color and A channel is
// used for variance. Separate textures are used for direct & indirect
// lighting.  An addition texture is used to represent the number of
// frames each pixel of history has been consistent and valid.
class ColorHistoryBuffer {
public:
  ColorHistoryBuffer(int width, int height);

  void bind() const;
  void unbind() const;
  unsigned int id() const;

  const CS123::GL::Texture2D &direct_texture() const;
  const CS123::GL::Texture2D &indirect_texture() const;
  const CS123::GL::Texture2D &history_length_texture() const;

private:
  unsigned int m_id;
  int m_width, m_height;

  CS123::GL::Texture2D m_direct, m_indirect, m_history_length;

  // Generate a texture of the given size and default params and
  // bind it to framebuffer.  Acceptable 'type's are GL_FLOAT and
  // GL_UNSIGNED_BYTE. (this either makes an RGBFloatTexture or an
  // UnsignedByteTexture as per CS123::GL::Texture2D.h)
  CS123::GL::Texture2D make_texture(GLenum type);
};

#endif // COLORHISTORYBUFFER_H
