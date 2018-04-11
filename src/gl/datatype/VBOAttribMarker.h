#ifndef VBOATTRIBMARKER_H
#define VBOATTRIBMARKER_H

#include <string>

#include "GL/glew.h"

namespace CS123 { namespace GL {

struct VBOAttribMarker {
    enum DATA_TYPE{ FLOAT = GL_FLOAT, INT = GL_INT, UNSIGNED_BYTE = GL_UNSIGNED_BYTE };
    enum DATA_NORMALIZE{ GLTRUE = GL_TRUE, GLFALSE = GL_FALSE };

    /**
     * @brief VBOAttribMarker
     * @param name OpenGL handle to the attribute location. These are specified in ShaderAttribLocations.h
     * @param numElementsPerVertex Number of elements per vertex. Must be 1, 2, 3 or 4 (e.g. position = 3 for x,y,z)
     * @param offset Offset in BYTES from the start of the array to the beginning of the first element
     * @param type Primitive type (FLOAT, INT, UNSIGNED_BYTE)
     * @param normalize
     */
    VBOAttribMarker(GLuint name, GLuint numElementsPerVertex, int offset, DATA_TYPE type = FLOAT, bool normalize = false);

    GLuint name;
    DATA_TYPE dataType;
    DATA_NORMALIZE dataNormalize;
    GLuint numElements;
    size_t offset;
};

}}

#endif // VBOATTRIBMARKER_H
