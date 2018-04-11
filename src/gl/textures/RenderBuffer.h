#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

namespace CS123 { namespace GL {

class RenderBuffer {
public:
    RenderBuffer();
    virtual ~RenderBuffer();
    RenderBuffer(const RenderBuffer &that) = delete;
    RenderBuffer& operator=(const RenderBuffer &that) = delete;
    RenderBuffer(RenderBuffer &&that);
    RenderBuffer& operator=(RenderBuffer &&that);


    void bind() const;
    unsigned int id() const;
    void unbind() const;

protected:
    unsigned int m_handle;
};

}}

#endif // RENDERBUFFER_H
