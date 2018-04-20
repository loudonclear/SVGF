#ifndef COLORVARIANCEBUFFER_H
#define COLORVARIANCEBUFFER_H


class ColorVarianceBuffer
{
public:
    ColorVarianceBuffer(int width, int height);

    void bind() const;
    void unbind() const;

    unsigned int getColorVarianceTexture() const;

private:
    unsigned int cvBuffer;
    unsigned int colorVariance;

    int m_width, m_height;
};
#endif // COLORVARIANCEBUFFER_H
