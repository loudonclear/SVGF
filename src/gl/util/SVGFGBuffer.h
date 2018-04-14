#ifndef SVGFGBUFFER_H
#define SVGFGBUFFER_H


class SVGFGBuffer
{
public:
    SVGFGBuffer(int width, int height);

    void bind() const;
    void unbind() const;

    void bindTextures() const;
    void depthBufferCopy() const;

private:
    unsigned int gBuffer;
    unsigned int gMeshMatID, gNormal;
    unsigned int rboDepth;

    int m_width, m_height;
};

#endif // SVGFGBUFFER_H
