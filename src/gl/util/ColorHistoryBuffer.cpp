#include "ColorHistoryBuffer.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

#include <iostream>

using namespace CS123::GL;

ColorHistoryBuffer::ColorHistoryBuffer(int width, int height)
  : Buffer(width, height),
    m_direct(this->makeTextureAndAttach(GL_FLOAT, 0)),
    m_indirect(this->makeTextureAndAttach(GL_FLOAT, 1)),
    m_history_length(this->makeTextureAndAttach(GL_UNSIGNED_BYTE, 2)) {
  bind();

  // attach textures to frame buffer
  unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                 GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);

  // set history length texture to 0
  m_history_length.bind();
  GLuint zero = 0;
  glClearBufferuiv(GL_COLOR, GL_COLOR_ATTACHMENT2, &zero);
  m_history_length.unbind();

  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete: " << status << std::endl;
  }

  unbind();
}

const Texture2D &ColorHistoryBuffer::direct_texture() const { return m_direct; }

const Texture2D &ColorHistoryBuffer::indirect_texture() const {
  return m_indirect;
}

const Texture2D &ColorHistoryBuffer::history_length_texture() const {
  return m_history_length;
}
