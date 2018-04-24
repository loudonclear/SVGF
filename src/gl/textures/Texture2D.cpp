#include "Texture2D.h"

#include <utility>

using namespace CS123::GL;

Texture2D::Texture2D(const GLvoid *data, int width, int height,
                     GLint internalFormat, GLenum format, GLenum type) {
  bind();
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type,
               data);
  unbind();
}

Texture2D Texture2D::RGBAFloatTex(const GLvoid *data, int width, int height) {
  return Texture2D(data, width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
}
Texture2D Texture2D::UnsignedByteTex(const GLvoid *data, int width,
                                     int height) {
  return Texture2D(data, width, height, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
}

void Texture2D::bind() const {
    glBindTexture(GL_TEXTURE_2D, m_handle);
}

void Texture2D::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
