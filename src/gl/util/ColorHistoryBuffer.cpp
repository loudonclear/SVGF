#include "ColorHistoryBuffer.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>

#include <iostream>

using namespace CS123::GL;


ColorHistoryBuffer::ColorHistoryBuffer(int width, int height) : m_width(width), m_height(height)
{
    glGenFramebuffers(1, &chBuffer);
    bind();

    m_direct = this->makeTexture(0, GL_FLOAT);
    m_indirect = this->makeTexture(1, GL_FLOAT);
    m_history_length = this->makeTexture(2, GL_UNSIGNED_BYTE);

    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    // set history length texture to 0
    m_history_length.bind();
    GLuint zero = 0;
    glClearBufferuiv(GL_COLOR, GL_COLOR_ATTACHMENT2, &zero);
    m_history_length.unbind();

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << status << std::endl;

    unbind();
}

void ColorHistoryBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, chBuffer);
    glViewport(0, 0, m_width, m_height);
}

void ColorHistoryBuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ColorHistoryBuffer::getDirectTexture() const {
    return m_direct;
}

unsigned int ColorHistoryBuffer::getIndirectTexture() const {
    return m_indirect;
}

unsigned int ColorHistoryBuffer::getHistoryLengthTexture() const {
    return m_history_length;
}

// TODO probably requires ColorHistoryBuffer to be bound
Texture2D ColorHistoryBuffer::makeTexture(unsigned int tex_id, GLenum type) {
  TextureParameters tex_params{GL_NEAREST, GL_CLAMP_TO_EDGE};
  Texture2D tex = (type == GL_FLOAT)
                      ? Texture2D::RGBAFloatTex(nullptr, m_width, m_height)
                      : Texture2D::UnsignedByteTex(nullptr, m_width, m_height);
  tex_parameters.applyTo(tex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + tex_id, GL_TEXTURE_2D,
                         tex.id(), 0);
  return tex;
}
