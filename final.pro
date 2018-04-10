# -------------------------------------------------
# Project created by QtCreator 2010-08-22T14:12:19
# -------------------------------------------------
QT += opengl xml
TARGET = final
TEMPLATE = app


QMAKE_CXXFLAGS += -msse2
QMAKE_CXXFLAGS += -std=c++14
CONFIG += c++14

# allow debug and release modes, and set -DNDEBUG for release mode
CONFIG += debug_and_release
CONFIG(release, debug|release): DEFINES += NDEBUG

Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui


win32 {
    DEFINES += GLEW_STATIC
    LIBS += -lopengl32 -lglu32
}

linux {
    # fixes an issue with linux and vr libraries
    QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
}

SOURCES += \
    libs/glew-1.10.0/src/glew.c \
    src/gl/shaders/Shader.cpp \
    src/gl/GLDebug.cpp \
    src/gl/datatype/VBOAttribMarker.cpp \
    src/gl/datatype/VBO.cpp \
    src/gl/datatype/IBO.cpp \
    src/gl/datatype/VAO.cpp \
    src/gl/datatype/FBO.cpp \
    src/gl/textures/Texture.cpp \
    src/gl/textures/Texture2D.cpp \
    src/gl/textures/TextureParameters.cpp \
    src/gl/textures/TextureParametersBuilder.cpp \
    src/gl/textures/RenderBuffer.cpp \
    src/gl/textures/DepthBuffer.cpp \
    src/gl/util/FullScreenQuad.cpp \
    src/ui/view.cpp \
    src/ui/viewformat.cpp \
    src/ui/mainwindow.cpp \
    src/main.cpp \
    src/scene/shape/mesh.cpp \
    src/scene/shape/triangle.cpp \
    src/scene/basiccamera.cpp \
    src/scene/scene.cpp \
    src/BVH/BBox.cpp \
    src/BVH/BVH.cpp \
    src/util/CS123XmlSceneParser.cpp \
    src/gl/util/SVGFGBuffer.cpp \
    src/pathtracer/pathtracer.cpp \
    src/gl/util/ResourceLoader.cpp \
    src/gl/util/openglshape.cpp


HEADERS += \
    libs/glew-1.10.0/include/GL/glew.h \
    src/gl/shaders/Shader.h \
    src/gl/GLDebug.h \
    src/gl/shaders/ShaderAttribLocations.h \
    src/gl/datatype/VBOAttribMarker.h \
    src/gl/datatype/VBO.h \
    src/gl/datatype/IBO.h \
    src/gl/datatype/VAO.h \
    src/gl/datatype/FBO.h \
    src/gl/textures/Texture.h \
    src/gl/textures/Texture2D.h \
    src/gl/textures/TextureParameters.h \
    src/gl/textures/TextureParametersBuilder.h \
    src/gl/textures/RenderBuffer.h \
    src/gl/textures/DepthBuffer.h \
    src/gl/util/FullScreenQuad.h \
    src/ui/view.h \
    src/ui/viewformat.h \
    src/ui/mainwindow.h \
    src/scene/shape/mesh.h \
    src/scene/shape/triangle.h \
    src/scene/basiccamera.h \
    src/scene/camera.h \
    src/scene/scene.h \
    src/BVH/BBox.h \
    src/BVH/BVH.h \
    src/BVH/IntersectionInfo.h \
    src/BVH/Log.h \
    src/BVH/Object.h \
    src/BVH/Ray.h \
    src/BVH/Stopwatch.h \
    src/BVH/vector3.h \
    src/util/CS123Common.h \
    src/util/CS123ISceneParser.h \
    src/util/CS123SceneData.h \
    src/util/CS123XmlSceneParser.h \
    src/util/tiny_obj_loader.h \
    src/gl/util/SVGFGBuffer.h \
    src/pathtracer/pathtracer.h \
    src/gl/util/ResourceLoader.h \
    src/gl/util/openglshape.h \
    src/gl/util/sphere.h



FORMS += src/ui/mainwindow.ui
INCLUDEPATH += libs src src/ui libs/glm libs/glew-1.10.0/include
DEPENDPATH += libs src src/ui libs/glm libs/glew-1.10.0/include

DEFINES += _USE_MATH_DEFINES
DEFINES += TIXML_USE_STL
DEFINES += GLM_SWIZZLE GLM_FORCE_RADIANS
OTHER_FILES += \
    shaders/*

RESOURCES += \
    resources.qrc


#defineTest(copyToDestdir) {
#    files = $$1
#    for(FILE, files) {
#        CONFIG(debug, debug|release) {
#            DDIR = $${OUT_PWD}/debug
#        } else {
#            DDIR = $${OUT_PWD}/release
#        }

#        # Replace slashes in paths with backslashes for Windows
#        win32:FILE ~= s,/,\\,g
#        win32:DDIR ~= s,/,\\,g
#        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
#    }
#    export(QMAKE_POST_LINK)
#}

#win32 {
#    contains(QT_ARCH, i386) {
#        message("32 bit build")
#            LIBS += -L$$PWD/libraries/openvr/lib/win32/ -lopenvr_api
#            copyToDestdir($$PWD/libraries/openvr/bin/win32/openvr_api.dll)
#    } else {
#        message("64 bit build")
#            LIBS += -L$$PWD/libraries/openvr/lib/win64/ -lopenvr_api
#            copyToDestdir($$PWD/libraries/openvr/bin/win64/openvr_api.dll)
#    }
#}

#linux {

#    LIBS += -L$$PWD/libraries/steam-runtime/amd64/lib/x86_64-linux-gnu
#    LIBS += -L$$PWD/libraries/steam-runtime/amd64/lib
#    LIBS += -L$$PWD/libraries/steam-runtime/amd64/usr/lib/x86_64-linux-gnu
#    LIBS += -L$$PWD/libraries/steam-runtime/amd64/usr/lib
#    LIBS += -L$$PWD/libraries/steam-runtime/i386/lib/i386-linux-gnu
#    LIBS += -L$$PWD/libraries/steam-runtime/i386/lib
#    LIBS += -L$$PWD/libraries/steam-runtime/i386/usr/lib/i386-linux-gnu
#    LIBS += -L$$PWD/libraries/steam-runtime/i386/usr/lib
#    LIBS += -L$$PWD/libraries/openvr/bin/linux64 -lopenvr_api
#}

DISTFILES += \
    src/BVH/License.txt \
    src/BVH/README \
    shaders/gbuffer.fsh
