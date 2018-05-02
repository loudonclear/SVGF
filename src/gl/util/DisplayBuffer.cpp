#include "DisplayBuffer.h"

#include "gl/textures/Texture2D.h"
#include "gl/textures/TextureParameters.h"

#include <GL/glew.h>
#include <iostream>

using namespace CS123::GL;

DisplayBuffer::DisplayBuffer(int width, int height) :
  Buffer(width, height),
  m_color(makeTextureAndAttach(GL_RGB16F, GL_RGB, GL_FLOAT, 0))
{
    bind();
    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << status << std::endl;
    unbind();


    TextureParameters tex_params{TextureParameters::FILTER_METHOD::LINEAR,
                                 TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE};

    tex_params.applyTo(color_texture());
}

const Texture2D& DisplayBuffer::color_texture() const {
    return m_color;
}
