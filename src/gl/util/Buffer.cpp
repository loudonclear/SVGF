#include "Buffer.h"

#include "gl/textures/Texture2D.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

using namespace CS123::GL;

Buffer::Buffer(int width, int height) : m_width(width), m_height(height) {
  glGenFramebuffers(1, &m_id);
}

Buffer::Buffer(int width, int height, int id) : m_width(width), m_height(height), m_id(id) {}

Buffer::~Buffer(){
  glDeleteFramebuffers(1, &m_id);
}

Buffer Buffer::default_buff(int width, int height){
  return Buffer(width, height, 0);
}

void Buffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  glViewport(0, 0, m_width, m_height);
}

void Buffer::unbind()  { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

unsigned int Buffer::id() const { return m_id; }

int Buffer::width() const {
  return m_width;
}

int Buffer::height() const {
  return m_height;
}

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

void Buffer::display(GLenum attachment) {
  Buffer::unbind();
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
  int old_read_attach;
  glGetIntegerv(GL_READ_BUFFER, &old_read_attach);
  glReadBuffer(attachment);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glReadBuffer(static_cast<GLenum>(old_read_attach));
  Buffer::unbind();
}

void Buffer::blit_to(GLenum in_attachment, Buffer& out_buff, GLenum out_attachment) const{
  Buffer::unbind();
  // set up read buffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
  int old_read_attach;
  glGetIntegerv(GL_READ_BUFFER, &old_read_attach);
  glReadBuffer(in_attachment);
  // set up draw buffer
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, out_buff.id());
  int old_draw_attach;
  glGetIntegerv(GL_READ_BUFFER, &old_draw_attach);
  glDrawBuffer(out_attachment);
  // do blit
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
  // cleanup
  glDrawBuffer(static_cast<GLenum>(old_draw_attach));
  glReadBuffer(static_cast<GLenum>(old_read_attach));
  Buffer::unbind();
}

void Buffer::blit_to(Buffer& out_buff) const {
  Buffer::blit_to(GL_COLOR_ATTACHMENT0, out_buff, GL_COLOR_ATTACHMENT0);
}

Texture2D Buffer::makeTexture(GLint internalFormat, GLenum format,
                              GLenum type, void *data) {
  TextureParameters tex_params{TextureParameters::FILTER_METHOD::NEAREST,
                               TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE};
  Texture2D tex =
      Texture2D(data, m_width, m_height, internalFormat, format, type);
  tex_params.applyTo(tex);
  return tex;
}

Texture2D Buffer::makeTexture(GLenum type) {
  GLint internalFormat;
  GLenum format;
  determineFormat(internalFormat, format, type);
  return makeTexture(internalFormat, format, type);
}

void Buffer::attachTexture(Texture2D& tex, unsigned int attach_id, bool do_unbind) {
  // attach to FBO
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach_id,
                         GL_TEXTURE_2D, tex.id(), 0);
  if (do_unbind) {
    Buffer::unbind();
  }
}

Texture2D Buffer::makeTextureAndAttach(GLint internalFormat, GLenum format,
                                       GLenum type, unsigned int attach_id,
                                       void *data, bool do_unbind) {
  Texture2D tex = makeTexture(internalFormat, format, type, data);
  attachTexture(tex, attach_id, do_unbind);
  return tex;
}

// Creates a texture, and optionally attaches it to the current framebuffer.
Texture2D Buffer::makeTextureAndAttach(GLenum type, unsigned int attach_id,
                                       bool do_unbind) {
  GLint internalFormat;
  GLenum format;
  determineFormat(internalFormat, format, type);
  return makeTextureAndAttach(internalFormat, format, type, attach_id,
                              nullptr, do_unbind);
}
