#include "view.h"

#include "pathtracer/pathtracer.h"
#include "scene/scene.h"
#include "scene/QuaternionCamera.h"
#include "viewformat.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>

#include <iostream>

View::View(QWidget *parent)
    : QGLWidget(ViewFormat(), parent), m_time(), m_timer(),
      m_captureMouse(false), m_scene(nullptr) {
  setMouseTracking(true);

  /* Hide the cursor */
  if (m_captureMouse) {
    QApplication::setOverrideCursor(Qt::BlankCursor);
  }

  /* View needs keyboard focus */
  setFocusPolicy(Qt::StrongFocus);

  /* The update loop is implemented using a timer */
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));

  /* Process Command Line Arguments */
  m_cli_parser.setApplicationDescription("Path tracer with render denoising using edge-avoiding wavelets.");
  m_cli_parser.addHelpOption();
  m_cli_parser.addPositionalArgument("scene", ".xml scene file to load for simulation");
  m_cli_parser.addOption({{"s", "samples"}, "Number of pathtracing samples", "samples"});
  m_cli_parser.addOption({{"a", "alpha"}, "How much to rely on current frame instead of history", "alpha", "0.2"});
  m_cli_parser.process(*QApplication::instance());
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
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.f, 0.f, 0.f, 1.f);

   /* Start a timer that will try to get 60 frames per second (the actual
    * frame rate depends on the operating system and other running programs)
    */
     m_time.start();
     m_timer.start(1000 / 60);

    /* Load initial scene  */
    const QStringList cli_args = m_cli_parser.positionalArguments();
    if (cli_args.size() > 0) {
        QFileInfo info(cli_args.at(0));
        QString absolute_scene_path = info.absoluteFilePath();
        m_scene = Scene::load(absolute_scene_path, width(), height());
        if (!m_scene) {
          std::cerr << "Error parsing scene file "
                    << absolute_scene_path.toStdString() << std::endl;
        }
        m_scene->pathTracer().numSamples(m_cli_parser.value("samples").toUInt());
        m_scene->integration_alpha() = m_cli_parser.value("alpha").toFloat();
    }
}

void View::change_settings(int renderMode, int numSamples, float alpha, bool temporalReprojection, int waveletIterations, float sigmaP, float sigmaN, float sigmaL, bool fxaa) {
    if (m_scene) {
        m_scene->change_settings(renderMode, numSamples, alpha, temporalReprojection, waveletIterations, sigmaP, sigmaN, sigmaL, fxaa);
    }
}

void View::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    if (m_scene) {
        m_scene->resize(w, h);
    }
}

void View::paintGL() {

    /* Render the selected scene */
    if (m_scene) {
        m_scene->render();
    }
}

void View::updateInputs(float dt) {
    if (!m_scene) return;

    const float moveSpeed = 1.5f;
    const float rotateSpeed = 50.f;

    QuaternionCamera c = m_scene->getCamera();
    if (m_keys[Qt::Key_A]) {
        c.translate(-c.getRight() * moveSpeed * dt);
    }
    if (m_keys[Qt::Key_D]) {
        c.translate(c.getRight() * moveSpeed * dt);
    }
    if (m_keys[Qt::Key_W]) {
        c.translate(c.getLook() * moveSpeed * dt);
    }
    if (m_keys[Qt::Key_S]) {
        c.translate(-c.getLook() * moveSpeed * dt);
    }
    if (m_keys[Qt::Key_Q]) {
        c.translate(-c.getUp() * moveSpeed * dt);
    }
    if (m_keys[Qt::Key_E]) {
        c.translate(c.getUp() * moveSpeed * dt);
    }


    if (m_keys[Qt::Key_Left]) {
        c.rotate(rotateSpeed * dt, glm::vec3(0, 1, 0));
    }
    if (m_keys[Qt::Key_Right]) {
        c.rotate(-rotateSpeed * dt, glm::vec3(0, 1, 0));
    }
    if (m_keys[Qt::Key_Up]) {
        c.rotate(rotateSpeed * dt, glm::vec3(1, 0, 0));
    }
    if (m_keys[Qt::Key_Down]) {
        c.rotate(-rotateSpeed * dt, glm::vec3(1, 0, 0));
    }
    m_scene->setCamera(c);
}

void View::tick() {
    /* Get the number of seconds since the last tick (variable update rate) */
    float dt = m_time.restart() * 0.001f;

    updateInputs(dt);

    /* Flag this view for repainting (Qt will call paintGL() soon after) */
    update();
}


/* Keyboard events */
void View::keyPressEvent(QKeyEvent *event) {
    m_keys[event->key()] = true;

    if (event->key() == Qt::Key_Escape) {
        QApplication::quit();
    }
    if (event->key() == Qt::Key_Space) {
      if (m_scene) m_scene->pipeline() = !m_scene->pipeline();
    }
}
void View::keyReleaseEvent(QKeyEvent *event) {
    m_keys[event->key()] = false;
}


/* Mouse events */
void View::mousePressEvent(QMouseEvent *) {}

void View::mouseReleaseEvent(QMouseEvent *) {}

void View::mouseMoveEvent(QMouseEvent *event) {
    if(m_captureMouse) {
        int deltaX = event->x() - width() / 2;
        int deltaY = event->y() - height() / 2;
        if (!deltaX && !deltaY) return;
        QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));

        // TODO: Handle mouse movements here
    }
}

void View::loadScene(QString file) {
    m_scene = Scene::load(file, width(), height());
    if (!m_scene) {
        std::cerr << "Error parsing scene file " << file.toStdString() << std::endl;
    }
}
