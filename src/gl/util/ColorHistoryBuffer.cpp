#include "ColorHistoryBuffer.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

#include <iostream>

using namespace CS123::GL;

ColorHistoryBuffer::ColorHistoryBuffer(int width, int height)
  : Buffer(width, height),
    m_color_history(this->makeTextureAndAttach(GL_RGBA16F, GL_RGBA, GL_FLOAT, 0)),
    m_moments(this->makeTextureAndAttach(GL_RG16F, GL_RG, GL_FLOAT, 1))
{
  bind();

  // attach textures to frame buffer
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  // set history length texture to 0
  m_color_history.bind();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  m_color_history.unbind();

  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete: " << status << std::endl;
  }

  Buffer::unbind();
}

const Texture2D &ColorHistoryBuffer::color_history() const { return m_color_history; }
const Texture2D &ColorHistoryBuffer::moments() const { return m_moments; }
