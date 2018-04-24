#include "GLDebug.h"

#include <iostream>
#include <vector>

#include <GL/glew.h>

namespace CS123 { namespace GL {

void checkError() {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "GL is in an error state before painting." << std::endl;
        printGLErrorCodeInEnglish(err);
    }
}

void printGLErrorCodeInEnglish(GLenum err) {
    std::cerr << "GL error code " << err << ":" << std::endl;
    switch(err) {
        case GL_NO_ERROR:
          std::cerr << "GL_NO_ERROR" << std::endl;
          std::cerr << "No Gl Error." << std::endl;
          break;
        case GL_INVALID_ENUM:
            std::cerr << "GL_INVALID_ENUM" << std::endl;
            std::cerr << "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cerr << "GL_INVALID_VALUE" << std::endl;
            std::cerr << "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cerr << "GL_INVALID_OPERATION" << std::endl;
            std::cerr << "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
            std::cerr << "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
            std::cerr << "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded." << std::endl;
            break;
        case GL_STACK_UNDERFLOW:
            std::cerr << "GL_STACK_UNDERFLOW" << std::endl;
            std::cerr << "An attempt has been made to perform an operation that would cause an internal stack to underflow." << std::endl;
            break;
        case GL_STACK_OVERFLOW:
            std::cerr << "GL_STACK_OVERFLOW" << std::endl;
            std::cerr << "An attempt has been made to perform an operation that would cause an internal stack to overflow." << std::endl;
            break;
        default:
            std::cerr << "Unknown GL error code" << std::endl;
    }
}

void checkFramebufferStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is incomplete." << std::endl;
        printFramebufferErrorCodeInEnglish(status);
    }
}

void printFramebufferErrorCodeInEnglish(GLenum err) {
    switch(err) {
        case GL_FRAMEBUFFER_UNDEFINED:
            std:: cerr << "GL_FRAMEBUFFER_UNDEFINED is returned if the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist." << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT is returned if any of the framebuffer attachment points are framebuffer incomplete." << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT is returned if the framebuffer does not have at least one image attached to it." << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi." << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER is returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER." << std::endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED is returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions." << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is returned if the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES." << std::endl;
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures." << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target." << std::endl;
            break;
    }
}

void checkShaderCompilationStatus(GLuint shaderID) {
    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        std::cerr << "Error: Could not compile shader."  << std::endl;

        GLint maxLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the null character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

        std::cerr << &errorLog[0] << std::endl;
        throw std::runtime_error("Shader failed to compile.");
    } else {
        std::cerr << "Shader compiled." << std::endl;
    }
}

void checkShaderLinkStatus(GLuint shaderProgramID) {
    GLint linked;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        std::cerr << "Shader failed to link" << std::endl;
        throw std::runtime_error("Shader failed to link.");
    } else {
        std::cerr << "Shader linked successfully." << std::endl;
    }
}

}}
