#ifndef VBO_H
#define VBO_H

#include <vector>

#include "GL/glew.h"

namespace CS123 { namespace GL {

class VBOAttribMarker;

class VBO {
public:
    enum GEOMETRY_LAYOUT { LAYOUT_TRIANGLES = GL_TRIANGLES,
                           LAYOUT_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
                           LAYOUT_TRIANGLE_FAN = GL_TRIANGLE_FAN,
                           LAYOUT_LINE_STRIP = GL_LINE_STRIP };

    /**
     * @brief VBO
     * @param data Pointer to the beginning of the data.
     * @param sizeInFloats Number of floats in the array.
     * @param markers List of VBOAttribMarkers that describe how the data is laid out.
     * @param layout Layout of the vertex data.
     */
    VBO(const float *data, int sizeInFloats, std::vector<VBOAttribMarker> markers, GEOMETRY_LAYOUT layout = LAYOUT_TRIANGLES);
    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;
    VBO(VBO &&that);
    VBO& operator=(VBO &&);
    ~VBO();

    void bindAndEnable() const;
    GEOMETRY_LAYOUT triangleLayout() const;
    int numberOfVertices() const;
    int numberOfFloatsPerVertex() const;

    void unbind() const;

private:
    void bind() const;

    GLuint m_handle;
    std::vector<VBOAttribMarker> m_markers;
    int m_bufferSizeInFloats;
    int m_numberOfFloatsPerVertex;
    GLuint m_stride;
    GEOMETRY_LAYOUT m_triangleLayout;
};

}}

#endif // VBO_H
