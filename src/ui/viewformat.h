#ifndef VIEWFORMAT_H
#define VIEWFORMAT_H

#include <QGLFormat>

/***
 * This is used to set the OpenGL version and core
 * profile in View's constructor for compatibility
 * reasons.
 ***/

class ViewFormat : public QGLFormat
{
public:
    ViewFormat();
};


#endif // VIEWFORMAT_H
