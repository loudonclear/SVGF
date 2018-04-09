#ifndef SVGFGBUFFER_H
#define SVGFGBUFFER_H


class SVGFGBuffer
{
public:
    SVGFGBuffer(int width, int height);

    inline void bind() const;
    inline void unbind() const;

    inline void bindTextures() const;

private:
    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedo;
    unsigned int rboDepth;

    int m_width, m_height;
};

#endif // SVGFGBUFFER_H
