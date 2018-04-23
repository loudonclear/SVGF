#ifndef COLORHISTORYBUFFER_H
#define COLORHISTORYBUFFER_H

#include "gl/textures/Texture2D.h"

// Reprents a GL FBO for storing color and variance history. Data is
// stored in RGBA format, where RGB represents color and A channel is
// used for variance. Separate textures are used for direct & indirect
// lighting.  An addition texture is used to represent the number of
// frames each pixel of history has been consistent and valid.
class ColorHistoryBuffer
{
public:
    ColorHistoryBuffer(int width, int height);

    void bind() const;
    void unbind() const;

    unsigned int getDirectTexture() const;
    unsigned int getIndirectTexture() const;
    unsigned int getHistoryLengthTexture() const;

private:
    unsigned int chBuffer;

    CS123::GL::Texture2D m_direct, m_indirect, m_history_length;

    int m_width, m_height;

    // Generate a texture of the given size and default params and
    // bind it to framebuffer.  Acceptable 'type's are GL_FLOAT and
    // GL_UNSIGNED_BYTE. (this either makes an RGBFloatTexture or an
    // UnsignedByteTexture as per Texture2D.h)
    CS123::GL::Texture2D makeTexture(unsigned int tex_id, GLenum type);
};

#endif // COLORHISTORYBUFFER_H
