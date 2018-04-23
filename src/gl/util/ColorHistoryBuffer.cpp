#include "ColorHistoryBuffer.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

#include <iostream>

using namespace CS123::GL;

ColorHistoryBuffer::ColorHistoryBuffer(int width, int height)
    : m_width(width), m_height(height), m_direct(this->make_texture(GL_FLOAT)),
      m_indirect(this->make_texture(GL_FLOAT)),
      m_history_length(this->make_texture(GL_UNSIGNED_BYTE)) {
  glGenFramebuffers(1, &m_id);
  bind();

  // attach textures to frame buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_direct.id(), 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         m_indirect.id(), 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         m_history_length.id(), 0);

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

void ColorHistoryBuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  glViewport(0, 0, m_width, m_height);
}

void ColorHistoryBuffer::unbind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ColorHistoryBuffer::id() const { return m_id; }

const Texture2D &ColorHistoryBuffer::direct_texture() const { return m_direct; }

const Texture2D &ColorHistoryBuffer::indirect_texture() const {
  return m_indirect;
}

const Texture2D &ColorHistoryBuffer::history_length_texture() const {
  return m_history_length;
}

// TODO probably requires ColorHistoryBuffer to be bound
Texture2D ColorHistoryBuffer::make_texture(GLenum type) {
  TextureParameters tex_params{TextureParameters::FILTER_METHOD::NEAREST,
                               TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE};
  Texture2D tex = (type == GL_FLOAT)
                      ? Texture2D::RGBAFloatTex(nullptr, m_width, m_height)
                      : Texture2D::UnsignedByteTex(nullptr, m_width, m_height);
  tex_params.applyTo(tex);
  return tex;
}
