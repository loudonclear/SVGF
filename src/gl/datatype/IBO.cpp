#include "IBO.h"

namespace CS123 { namespace GL {

IBO::IBO(int *data, int size) :
    m_handle(-1)
{
    glGenBuffers(1, &m_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(int), &(data[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IBO::~IBO()
{
    glDeleteBuffers(1, &m_handle);
}

void IBO::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle);
}

void IBO::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}}
