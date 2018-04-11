#ifndef VAO_H
#define VAO_H

#include <memory>

#include "GL/glew.h"

namespace CS123 { namespace GL {

class VBO;
class IBO;

class VAO {
public:
    VAO(const VBO &vbo, int numberOfVerticesToRender = 0);
    VAO(const VBO &vbo, const IBO &ibo, int numberOfVerticesToRender = 0);
    VAO(const VAO &that) = delete;
    VAO& operator=(const VAO &that) = delete;
    VAO(VAO &&that);
    VAO& operator=(VAO &&that);
    ~VAO();

    enum DRAW_METHOD { DRAW_ARRAYS, DRAW_INDEXED };

    void bind();
    void draw();
    void draw(int count);
    DRAW_METHOD drawMethod();
    void unbind();

private:
    std::unique_ptr<VBO> m_VBO;

    DRAW_METHOD m_drawMethod;
    GLuint m_handle;
    GLuint m_numVertices;
    int m_size;
    GLenum m_triangleLayout;
//    VBO::GEOMETRY_LAYOUT m_triangleLayout;
};

}}

#endif // VAO_H
