#ifndef RESULTBUFFER_H
#define RESULTBUFFER_H


#include "pathtracer/pathtracer.h"

class ResultBuffer
{
public:
    ResultBuffer(int width, int height);

    void bind() const;
    void unbind() const;

    unsigned int getColorTexture() const;


private:
    unsigned int cBuffer;
    unsigned int color;

    int m_width, m_height;
};

#endif // RESULTBUFFER_H
