#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "GL/glew.h"
#include "glm/glm.hpp"

namespace CS123 { namespace GL {

class Texture1D;
class Texture2D;
class Texture3D;
class TextureCube;

class Shader {
public:
    Shader(const std::string &vertexSource, const std::string &fragmentSource);
    Shader(const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource);

    Shader(Shader &that) = delete;
    Shader& operator=(Shader &that) = delete;
    virtual ~Shader();
    Shader(Shader &&that);
    Shader& operator=(Shader &&that);


    void setUniform(const std::string &name, float f);
    void setUniform(const std::string &name, const glm::vec2 &vec2);
    void setUniform(const std::string &name, const glm::vec3 &vec3);
    void setUniform(const std::string &name, const glm::vec4 &vec4);
    void setUniform(const std::string &name, int i);
    void setUniform(const std::string &name, const glm::ivec2 &ivec2);
    void setUniform(const std::string &name, const glm::ivec3 &ivec3);
    void setUniform(const std::string &name, const glm::ivec4 &ivec4);
    void setUniform(const std::string &name, bool b);
    void setUniform(const std::string &name, const glm::bvec2 &bvec2);
    void setUniform(const std::string &name, const glm::bvec3 &bvec3);
    void setUniform(const std::string &name, const glm::bvec4 &bvec4);
    void setUniform(const std::string &name, const glm::mat2 &mat2);
    void setUniform(const std::string &name, const glm::mat3 &mat3);
    void setUniform(const std::string &name, const glm::mat4 &mat4);

    void setUniformArrayByIndex(const std::string &name, float f, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::vec2 &vec2, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::vec3 &vec3, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::vec4 &vec4, size_t index);
    void setUniformArrayByIndex(const std::string &name, int i, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::ivec2 &ivec2, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::ivec3 &ivec3, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::ivec4 &ivec4, size_t index);
    void setUniformArrayByIndex(const std::string &name, bool b, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::bvec2 &bvec2, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::bvec3 &bvec3, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::bvec4 &bvec4, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::mat2 &mat2, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::mat3 &mat3, size_t index);
    void setUniformArrayByIndex(const std::string &name, const glm::mat4 &mat4, size_t index);

    void setTexture(const std::string &name, const Texture1D &t);
    void setTexture(const std::string &name, const Texture2D &t);
    void setTexture(const std::string &name, const Texture3D &t);
    void setTexture(const std::string &name, const TextureCube &t);

    void bind();
    void unbind();
    GLuint getID() const { return m_programID; }

private:

    GLuint createFragmentShaderFromSource(const std::string &source);
    GLuint createGeometryShaderFromSource(const std::string &source);
    void compileShader(GLuint handle, const std::string &source);
    GLuint createVertexShaderFromSource(const std::string &source);
    GLuint createShaderFromSource(const std::string &source, GLenum shaderType);

    void createProgramID();
    void attachShaders(const std::vector<GLuint> &shaders);
    void buildShaderProgramFromShaders(const std::vector<GLuint> &shaders);
    void linkShaderProgram();
    void detachShaders(const std::vector<GLuint> &shaders);
    void deleteShaders(const std::vector<GLuint> &shaders);

    void discoverShaderData();
    void discoverAttributes();
    void discoverUniforms();

    bool isUniformArray(const GLchar *name , GLsizei nameLength);
    bool isTexture(GLenum type);
    void addUniform(const std::string &name);
    void addUniformArray(const std::string &name, size_t size);
    void addTexture(const std::string &name);
    GLuint m_programID;

    std::map<std::string, GLuint> m_attributes;
    std::map<std::string, GLuint> m_uniforms;
    std::map<std::tuple<std::string, size_t>, GLuint> m_uniformArrays;
    std::map<std::string, GLuint> m_textureLocations; // name to uniform location
    std::map<GLuint, GLuint> m_textureSlots; // uniform location to texture slot
};

}}

#endif // SHADER_H
