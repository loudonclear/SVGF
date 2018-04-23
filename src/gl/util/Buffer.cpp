#include "Buffer.h"

#include "gl/textures/Texture2D.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

using namespace CS123::GL;

Buffer::Buffer(int width, int height) : m_width(width), m_height(height) {
  glGenFramebuffers(1, &m_id);
}

void Buffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  glViewport(0, 0, m_width, m_height);
}

void Buffer::unbind()  { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

unsigned int Buffer::id() const { return m_id; }

// helper function for choosing between two common types of textures
// (16-bit float RBGA and uint8 array) 16-bit float is returned if
// type == GL_FLOAT, uint8 is returned otherwise.
void determineFormat(GLint &internalFormat, GLenum &format, GLenum &type) {
  if (type == GL_FLOAT) {
    internalFormat = GL_RGBA16F;
    format = GL_RGBA;
    type = GL_FLOAT;
  } else {
    internalFormat = GL_R8;
    format = GL_RED;
    type = GL_UNSIGNED_BYTE;
  }
}

Texture2D Buffer::makeTexture(GLint internalFormat, GLenum format,
                              GLenum type) {
  TextureParameters tex_params{TextureParameters::FILTER_METHOD::NEAREST,
                               TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE};
  Texture2D tex =
      Texture2D(nullptr, m_width, m_height, internalFormat, format, type);
  tex_params.applyTo(tex);
  return tex;
}

Texture2D Buffer::makeTexture(GLenum type) {
  GLint internalFormat;
  GLenum format;
  determineFormat(internalFormat, format, type);
  return makeTexture(internalFormat, format, type);
}

Texture2D Buffer::makeTextureAndAttach(GLint internalFormat, GLenum format,
                                       GLenum type, unsigned int attach_id,
                                       bool do_unbind) {
  Texture2D tex = makeTexture(internalFormat, format, type);
  // attach to FBO
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach_id,
                         GL_TEXTURE_2D, tex.id(), 0);
  if (do_unbind) {
    unbind();
  }
  return tex;
}

// Creates a texture, and optionally attaches it to the current framebuffer.
Texture2D Buffer::makeTextureAndAttach(GLenum type, unsigned int attach_id,
                                       bool do_unbind) {
  GLint internalFormat;
  GLenum format;
  determineFormat(internalFormat, format, type);
  return makeTextureAndAttach(internalFormat, format, type, attach_id,
                              do_unbind);
}
