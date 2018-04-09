#include "viewformat.h"

ViewFormat::ViewFormat()
{
    setVersion(4, 3);
    setProfile(QGLFormat::CoreProfile);
    setSampleBuffers(true);
}
