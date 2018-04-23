#ifndef COLORHISTORYBUFFER_H
#define COLORHISTORYBUFFER_H

#include "gl/util/Buffer.h"

#include "gl/textures/Texture2D.h"

// Reprents a GL FBO for storing color and variance history. Data is
// stored in RGBA format, where RGB represents color and A channel is
// used for variance. Separate textures are used for direct & indirect
// lighting.  An addition texture is used to represent the number of
// frames each pixel of history has been consistent and valid.
class ColorHistoryBuffer : Buffer {
public:
  ColorHistoryBuffer(int width, int height);

  const CS123::GL::Texture2D &direct_texture() const;
  const CS123::GL::Texture2D &indirect_texture() const;
  const CS123::GL::Texture2D &history_length_texture() const;

private:
  CS123::GL::Texture2D m_direct, m_indirect, m_history_length;
};

#endif // COLORHISTORYBUFFER_H
