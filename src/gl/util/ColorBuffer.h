#ifndef COLORBUFFER_H
#define COLORBUFFER_H

#include "pathtracer/pathtracer.h"

class ColorBuffer
{
public:
    ColorBuffer(int width, int height, RenderBuffers &buffers);

    void bind() const;
    void unbind() const;

    unsigned int getDirectTexture() const;
    unsigned int getIndirectTexture() const;
    unsigned int getAlbedoTexture() const;


private:
    unsigned int cvBuffer;
    unsigned int direct, indirect, albedo;

    int m_width, m_height;
};
#endif // COLORBUFFER_H
