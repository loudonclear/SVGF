#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <exception>
#include <string>

#include "GL/glew.h"

class IOException : public std::exception {
public:
    IOException(const std::string &what) : message(what) {}
    virtual ~IOException() throw() {}
    virtual const char* what() const throw() override { return message.c_str(); }

private:
    std::string message;
};

class ResourceLoader
{
public:
    static std::string loadResourceFileToString(const std::string &resourcePath);
};

#endif // RESOURCELOADER_H
