#include "DepthBuffer.h"

#include "GL/glew.h"


DepthBuffer::DepthBuffer(int width, int height) :
    m_width(width),
    m_height(height)
{
    bind();
    glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    unbind();
}
