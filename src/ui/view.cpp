#include "view.h"
#include "viewformat.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <iostream>

#include "scene/scene.h"

View::View(QWidget *parent)
    : QGLWidget(ViewFormat(), parent),
    m_time(),
    m_timer(),
    m_captureMouse(false),
    m_scene(nullptr)
{
    setMouseTracking(true);

    /* Hide the cursor */
    if (m_captureMouse) {
        QApplication::setOverrideCursor(Qt::BlankCursor);
    }

    /* View needs keyboard focus */
    setFocusPolicy(Qt::StrongFocus);

    /* The update loop is implemented using a timer */
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
}

View::~View()
{
    if (m_scene) delete m_scene;
}

void View::initializeGL() {
   /* All OpenGL initialization *MUST* be done during or after this
    * method. Before this method is called, there is no active OpenGL
    * context and all OpenGL calls have no effect.
    */

    /* Initialize glew */
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cerr << "Something is very wrong, glew initialization failed." << std::endl;
    }
    std::cout << "Using GLEW " <<  glewGetString( GLEW_VERSION ) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.f, 0.f, 0.f, 1.f);

   /* Start a timer that will try to get 60 frames per second (the actual
    * frame rate depends on the operating system and other running programs)
    */
    m_time.start();
    m_timer.start(1000 / 60);
}

void View::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void View::paintGL() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Render the selected scene */
    if (m_scene) {
        m_scene->render();
    }
}

void View::updateInputs() {

    // TODO: Handle Input
}

void View::tick() {
    /* Get the number of seconds since the last tick (variable update rate) */
    float dt = m_time.restart() * 0.001f;

    updateInputs();

    /* Flag this view for repainting (Qt will call paintGL() soon after) */
    update();
}


/* Keyboard events */
void View::keyPressEvent(QKeyEvent *event) {
    m_keys[event->key()] = true;
}
void View::keyReleaseEvent(QKeyEvent *event) {
    m_keys[event->key()] = false;
}


/* Mouse events */
void View::mousePressEvent(QMouseEvent *event) {}

void View::mouseReleaseEvent(QMouseEvent *event) {}

void View::mouseMoveEvent(QMouseEvent *event) {
    if(m_captureMouse) {
        int deltaX = event->x() - width() / 2;
        int deltaY = event->y() - height() / 2;
        if (!deltaX && !deltaY) return;
        QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));

        // TODO: Handle mouse movements here
    }
}


/* File dialog */
void View::fileOpen() {
    QString file = QFileDialog::getOpenFileName(this, "Open File", "./res/scenes/");
    if (!file.isNull()) {

        delete m_scene;
        m_scene = nullptr;
        if(!Scene::load(file, &m_scene, width(), height())) {
            std::cerr << "Error parsing scene file " << file.toStdString() << std::endl;
        }
    }
}
