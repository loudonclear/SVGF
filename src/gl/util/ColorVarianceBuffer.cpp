#include "ColorVarianceBuffer.h"

#include <iostream>
#include <GL/glew.h>

ColorVarianceBuffer::ColorVarianceBuffer(int width, int height) : m_width(width), m_height(height)
{
    glGenFramebuffers(1, &cvBuffer);
    bind();

    // Color, Variance Buffer
    glGenTextures(1, &colorVariance);
    glBindTexture(GL_TEXTURE_2D, colorVariance);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorVariance, 0);

    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << status << std::endl;

    unbind();
}

void ColorVarianceBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, cvBuffer);
    glViewport(0, 0, m_width, m_height);
}

void ColorVarianceBuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ColorVarianceBuffer::getColorVarianceTexture() const {
    return colorVariance;
}
