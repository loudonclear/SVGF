#include "TextureParametersBuilder.h"

using namespace CS123::GL;

TextureParametersBuilder::TextureParametersBuilder() :
    m_filterMethod(TextureParameters::FILTER_METHOD::NEAREST),
    m_wrapMethod(TextureParameters::WRAP_METHOD::REPEAT)
{
}

void TextureParametersBuilder::setFilter(TextureParameters::FILTER_METHOD filterMethod) {
    m_filterMethod = filterMethod;
}

void TextureParametersBuilder::setWrap(TextureParameters::WRAP_METHOD wrapMethod) {
    m_wrapMethod = wrapMethod;
}

TextureParameters TextureParametersBuilder::build() {
    return TextureParameters(m_filterMethod, m_wrapMethod);
}
