#ifndef COLORHISTORYBUFFER_H
#define COLORHISTORYBUFFER_H

#include "gl/util/Buffer.h"

#include "gl/textures/Texture2D.h"

// Reprents a GL FBO for storing color and variance history.
class ColorHistoryBuffer : public Buffer {
public:
  ColorHistoryBuffer(int width, int height);

  const CS123::GL::Texture2D &color_history() const;
  const CS123::GL::Texture2D &moments() const;

private:
  // Color History is stored as float RGBA where A is history length.
  // Moments are stored as pairs of floats
  CS123::GL::Texture2D m_color_history, m_moments;
};

#endif // COLORHISTORYBUFFER_H
