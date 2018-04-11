#ifndef TEXTUREPARAMETERS_H
#define TEXTUREPARAMETERS_H

#include "GL/glew.h"

namespace CS123 { namespace GL {

class Texture2D;

class TextureParameters {
public:

    enum class FILTER_METHOD { NEAREST = GL_NEAREST,
                               LINEAR = GL_LINEAR };

    enum class WRAP_METHOD { CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
                             REPEAT = GL_REPEAT };

    void applyTo(const Texture2D &texture);

    friend class TextureParametersBuilder;

private:
    TextureParameters(FILTER_METHOD filterMethod, WRAP_METHOD wrapMethod);

    FILTER_METHOD m_filterMethod;
    WRAP_METHOD m_wrapMethod;
};

}}

#endif // TEXTUREPARAMETERS_H
