#ifndef TEXTURE_H
#define TEXTURE_H

namespace CS123 { namespace GL {

class Texture {
public:
    Texture();
    virtual ~Texture();
    Texture(const Texture &that) = delete;
    Texture& operator=(const Texture &that) = delete;
    Texture(Texture &&that);
    Texture& operator=(Texture &&that);


    virtual void bind() const = 0;
    unsigned int id() const;
    virtual void unbind() const = 0;

protected:
    unsigned int m_handle;

    // delete the underlying texture
    void clean();
};

}}

#endif // TEXTURE_H
