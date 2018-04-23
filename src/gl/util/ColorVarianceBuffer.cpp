#include "ColorVarianceBuffer.h"

#include "Buffer.h"

#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

#include <iostream>

using namespace CS123::GL;

ColorVarianceBuffer::ColorVarianceBuffer(int width, int height)
    : Buffer(width, height),
      m_colorVariance(this->makeTextureAndAttach(GL_FLOAT, 0)) {
  bind();

  unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);

  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete: " << status << std::endl;

  Buffer::unbind();
}

const Texture2D &ColorVarianceBuffer::color_variance_texture() const {
  return m_colorVariance;
}
