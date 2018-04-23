#ifndef COLORVARIANCEBUFFER_H
#define COLORVARIANCEBUFFER_H

#include "Buffer.h"

#include "gl/textures/Texture2D.h"

class ColorVarianceBuffer : public Buffer
{
public:
    ColorVarianceBuffer(int width, int height);
    const CS123::GL::Texture2D& color_variance_texture() const;

private:
    CS123::GL::Texture2D m_colorVariance;
};
#endif // COLORVARIANCEBUFFER_H
