#ifndef DEPTHBUFFER_H
#define DEPTHBUFFER_H

#include "RenderBuffer.h"

class DepthBuffer : public RenderBuffer {
public:
    DepthBuffer(int width, int height);

private:
    int m_width;
    int m_height;
};

#endif // DEPTHBUFFER_H
