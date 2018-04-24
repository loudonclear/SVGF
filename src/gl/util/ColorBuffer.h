#ifndef COLORBUFFER_H
#define COLORBUFFER_H

#include "Buffer.h"

#include "gl/textures/Texture2D.h"
#include "pathtracer/pathtracer.h"

class ColorBuffer : public Buffer
{
public:
    ColorBuffer(int width, int height, RenderBuffers &buffers);

    const CS123::GL::Texture2D &getDirectTexture() const;
    const CS123::GL::Texture2D &getIndirectTexture() const;
    const CS123::GL::Texture2D &getAlbedoTexture() const;

private:
    CS123::GL::Texture2D m_direct, m_indirect, m_albedo;
};
#endif // COLORBUFFER_H
