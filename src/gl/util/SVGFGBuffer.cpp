#include "SVGFGBuffer.h"

#include <GL/glew.h>
#include <iostream>

using namespace CS123::GL;

SVGFGBuffer::
SVGFGBuffer(int width, int height)
    : Buffer(width, height),
      gPositionMeshID(Buffer::makeTextureAndAttach(GL_RGBA16F, GL_RGBA, GL_FLOAT, 0, 0)),
      gNormal(Buffer::makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 1, 0))

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

const Texture2D &SVGFGBuffer::position_mesh_id_texture() const { return gPositionMeshID; }

const Texture2D &SVGFGBuffer::normal_texture() const { return gNormal; }

void SVGFGBuffer::set_textures(const RenderBuffers& buffers) {
  gPositionMeshID = Buffer::makeTextureAndAttach(GL_RGBA16F, GL_RGBA, GL_FLOAT, 0, buffers.interleave_pos_id().get());
  gNormal = Buffer::makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 1, buffers.l(RenderBuffers::NORMALS).get());
}
