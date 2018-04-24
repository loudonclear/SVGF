#include "ColorBuffer.h"

#include <iostream>
#include <GL/glew.h>

using namespace CS123::GL;

ColorBuffer::ColorBuffer(int width, int height, RenderBuffers &buffers) :
  Buffer(width, height),
  m_direct(makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 0, buffers.m_direct.get())),
  m_indirect(makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 1, buffers.m_indirect.get())),
  m_albedo(makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 2, buffers.m_albedo.get()))
{
    bind();

    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << status << std::endl;

    Buffer::unbind();
}

const Texture2D& ColorBuffer::getDirectTexture() const {
    return m_direct;
}

const Texture2D&  ColorBuffer::getIndirectTexture() const {
    return m_indirect;
}

const Texture2D&  ColorBuffer::getAlbedoTexture() const {
    return m_albedo;
}
