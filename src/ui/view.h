#ifndef VIEW_H
#define VIEW_H

#include "glm/glm.hpp"
#include "GL/glew.h"
#include "scene/scene.h"

#include <qgl.h>

#include <QTime>
#include <QTimer>
#include <QCommandLineParser>

#include <memory>
#include <unordered_map>


class View : public QGLWidget {
    Q_OBJECT

public:
    View(QWidget *parent);

    void change_settings(int renderMode, int numSamples, int maxDepth, float alpha, bool temporalReprojection, int waveletIterations, float sigmaP, float sigmaN, float sigmaL, bool fxaa);
    void loadScene(QString file);

private:
    QCommandLineParser m_cli_parser;
    QTime m_time;
    QTimer m_timer;
    bool m_captureMouse;
    std::unordered_map<int, bool> m_keys;

    std::unique_ptr<Scene> m_scene;

    // initialized openGL
    void initializeGL();

    void paintGL();

    void resizeGL(int w, int h);

    void updateInputs(float dt);

    // mouse and keyboard events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);


private slots:
    void tick();
};

#endif // VIEW_H
