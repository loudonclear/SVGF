#include "ResourceLoader.h"
#include <QFile>
#include <QString>
#include <QTextStream>

std::string ResourceLoader::loadResourceFileToString(const std::string &resourcePath)
{
    QString vertFilePath = QString::fromStdString(resourcePath);
    QFile vertFile(vertFilePath);
    if (vertFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream vertStream(&vertFile);
        QString contents = vertStream.readAll();
        return contents.toStdString();
    }
    throw IOException("Could not open file: " + resourcePath);
}

GLuint ResourceLoader::createShaderProgram(const char *vertexFilePath,const char *fragmentFilePath) {
    // Create and compile the shaders.
    GLuint vertexShaderID = createShader(GL_VERTEX_SHADER, vertexFilePath);
    GLuint fragmentShaderID = createShader(GL_FRAGMENT_SHADER, fragmentFilePath);

    // Link the shader program.
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderID);
    glAttachShader(programId, fragmentShaderID);
    glLinkProgram(programId);

    // Print the info log.
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetProgramiv(programId, GL_LINK_STATUS, &result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> infoLog(std::max(infoLogLength, int(1)));
        glGetProgramInfoLog(programId, infoLogLength, NULL, &infoLog[0]);
        fprintf(stdout, "%s\n", &infoLog[0]);
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programId;
}

GLuint ResourceLoader::createShader(GLenum shaderType, const char *filepath) {
    GLuint shaderID = glCreateShader(shaderType);

    // Read shader file.
    std::string code;
    QString filepathStr = QString(filepath);
    QFile file(filepathStr);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        code = stream.readAll().toStdString();
    }

    // Compile shader code.
    printf("Compiling shader: %s\n", filepath);
    const char *codePtr = code.c_str();
    glShaderSource(shaderID, 1, &codePtr, NULL);
    glCompileShader(shaderID);

    // Print info log.
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(shaderID, infoLogLength, NULL, &infoLog[0]);
        fprintf(stdout, "%s\n", &infoLog[0]);
    }

    return shaderID;
}
