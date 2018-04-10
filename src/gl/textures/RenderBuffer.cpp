#include "RenderBuffer.h"

#include "GL/glew.h"

using namespace CS123::GL;

RenderBuffer::RenderBuffer() :
    m_handle(0)
{
    glGenRenderbuffers(1, &m_handle);
}

RenderBuffer::RenderBuffer(RenderBuffer &&that) :
    m_handle(that.m_handle)
{
    that.m_handle = 0;
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer &&that) {
    this->~RenderBuffer();
    m_handle = that.m_handle;
    that.m_handle = 0;
    return *this;
}

RenderBuffer::~RenderBuffer()
{
    glDeleteRenderbuffers(1, &m_handle);
}

void RenderBuffer::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, m_handle);
}

unsigned int RenderBuffer::id() const {
    return m_handle;
}

void RenderBuffer::unbind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
