#include "ResultBuffer.h"

#include <iostream>
#include <GL/glew.h>

ResultBuffer::ResultBuffer(int width, int height) : m_width(width), m_height(height)
{
    glGenFramebuffers(1, &cBuffer);
    bind();

    // color Color
    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << status << std::endl;

    unbind();
}

void ResultBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, cBuffer);
    glViewport(0, 0, m_width, m_height);
}

void ResultBuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ResultBuffer::getColorTexture() const {
    return color;
}
