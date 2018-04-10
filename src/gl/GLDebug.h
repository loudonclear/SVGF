#ifndef GLDEBUG_H
#define GLDEBUG_H

#include "GL/glew.h"

namespace CS123 { namespace GL {

    void checkError();
    void printGLErrorCodeInEnglish(GLenum err);

    void checkFramebufferStatus();
    void printFramebufferErrorCodeInEnglish(GLenum err);

    void checkShaderCompilationStatus(GLuint shaderID);
    void checkShaderLinkStatus(GLuint shaderProgramID);

}}

#endif // GLDEBUG_H
