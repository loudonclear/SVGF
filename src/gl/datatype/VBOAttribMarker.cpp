#include "VBOAttribMarker.h"

namespace CS123 { namespace GL {

VBOAttribMarker::VBOAttribMarker(GLuint name, GLuint numElementsPerVertex, int offset, DATA_TYPE type , bool normalize) :
    name(name),
    dataType(type),
    dataNormalize(normalize ? GLTRUE : GLFALSE),
    numElements(numElementsPerVertex),
    offset(offset)
{
}

}}
