#ifndef VIEW_H
#define VIEW_H

#include "glm/glm.hpp"
#include "GL/glew.h"

#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <memory>
#include <unordered_map>

class Scene;

class View : public QGLWidget {
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

private:
    QTime m_time;
    QTimer m_timer;
    bool m_captureMouse;
    std::unordered_map<int, bool> m_keys;

    Scene* m_scene;

    // initialized openGL
    void initializeGL();

    void paintGL();

    void resizeGL(int w, int h);

    void updateInputs();

    // mouse and keyboard events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void fileOpen();

private slots:
    void tick();
};

#endif // VIEW_H
