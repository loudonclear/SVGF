#ifndef IBO_H
#define IBO_H

#include "GL/glew.h"

namespace CS123 { namespace GL {

class IBO {
public:
    IBO(int* data, int size);
    ~IBO();

    void bind() const;
    void unbind() const;


private:
    GLuint m_handle;
};

}}

#endif // IBO_H
