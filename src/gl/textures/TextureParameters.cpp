#include "TextureParameters.h"

#include "gl/GLDebug.h"
#include "gl/textures/Texture.h"
#include "gl/textures/Texture2D.h"

namespace CS123 { namespace GL {


TextureParameters::TextureParameters(FILTER_METHOD filterMethod, WRAP_METHOD wrapMethod) :
    m_filterMethod(filterMethod),
    m_wrapMethod(wrapMethod)
{
}

void TextureParameters::applyTo(const Texture2D &texture) {
    texture.bind();
    GLenum filterEnum = (GLenum)m_filterMethod;
    GLenum wrapEnum = (GLenum)m_wrapMethod;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterEnum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterEnum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapEnum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapEnum);

    texture.unbind();
}

}}
