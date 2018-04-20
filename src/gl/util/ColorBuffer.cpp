#include "ColorBuffer.h"

#include <iostream>
#include <GL/glew.h>

ColorBuffer::ColorBuffer(int width, int height, RenderBuffers &buffers) : m_width(width), m_height(height)
{
    glGenFramebuffers(1, &cBuffer);
    bind();

    // Direct Color
    glGenTextures(1, &direct);
    glBindTexture(GL_TEXTURE_2D, direct);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, static_cast<const void *>(buffers.m_direct.get()));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, direct, 0);

    // Indirect Color
    glGenTextures(1, &indirect);
    glBindTexture(GL_TEXTURE_2D, indirect);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, static_cast<const void *>(buffers.m_indirect.get()));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, indirect, 0);

    // Albedo
    glGenTextures(1, &albedo);
    glBindTexture(GL_TEXTURE_2D, albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, static_cast<const void *>(buffers.m_albedo.get()));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo, 0);

    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << status << std::endl;

    unbind();
}

void ColorBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, cBuffer);
    glViewport(0, 0, m_width, m_height);
}

void ColorBuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ColorBuffer::getDirectTexture() const {
    return direct;
}

unsigned int ColorBuffer::getIndirectTexture() const {
    return indirect;
}

unsigned int ColorBuffer::getAlbedoTexture() const {
    return albedo;
}
