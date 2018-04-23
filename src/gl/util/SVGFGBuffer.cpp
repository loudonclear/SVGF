#include "SVGFGBuffer.h"

#include <GL/glew.h>
#include <iostream>

using namespace CS123::GL;

SVGFGBuffer::SVGFGBuffer(int width, int height)
    : Buffer(width, height),
      gDepthIDs(makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 0)),
      gNormal(makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 1))

{
  bind();

  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rboDepth);

  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete: " << status << std::endl;
  }

  Buffer::unbind();
}

const Texture2D &SVGFGBuffer::depth_ids_texture() const { return gDepthIDs; }

const Texture2D &SVGFGBuffer::normal_texture() const { return gNormal; }

// void SVGFGBuffer::depth_buffer_copy() const {
//     glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
//     glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//     glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
//     GL_DEPTH_BUFFER_BIT, GL_NEAREST);
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }
