#ifndef TEXTUREPARAMETERSBUILDER_H
#define TEXTUREPARAMETERSBUILDER_H

#include "TextureParameters.h"

namespace CS123 { namespace GL {

class TextureParametersBuilder {
public:
    TextureParametersBuilder();

    void setFilter(TextureParameters::FILTER_METHOD filterMethod);
    void setWrap(TextureParameters::WRAP_METHOD wrapMethod);

    TextureParameters build();

private:
    TextureParameters::FILTER_METHOD m_filterMethod;
    TextureParameters::WRAP_METHOD m_wrapMethod;
};

}}

#endif // TEXTUREPARAMETERSBUILDER_H
