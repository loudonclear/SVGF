#include <QApplication>
#include "src/ui/mainwindow.h"

#include <cstdlib>
#include <ctime>

#include <iostream>
#include "glm.hpp"
#include "glm/gtx/string_cast.hpp"

int main(int argc, char *argv[])
{
    std::vector<glm::vec3> test;
    test.push_back(glm::vec3(0.3f, 0.4f, 0.5f)); test.push_back(glm::vec3(0.2f, 0.8f, 0.2f)); test.push_back(glm::vec3(0.0f, 0.8f, 0.4f));
    test.push_back(glm::vec3(0.9f, 0.6f, 0.3f)); test.push_back(glm::vec3(0.3f, 0.3f, 0.7f)); test.push_back(glm::vec3(0.2f, 0.7f, 0.1f));
    test.push_back(glm::vec3(0.9f, 0.5f, 0.1f)); test.push_back(glm::vec3(0.1f, 0.6f, 0.8f)); test.push_back(glm::vec3(0.4f, 0.0f, 0.0f));

    float kernel[9] = {1.f/16.f, 1.f/8.f, 1.f/16.f,
                       1.f/8.f,  1.f/4.f, 1.f/8.f,
                       1.f/16.f, 1.f/8.f, 1.f/16.f};

    glm::vec3 res = glm::vec3(0.f);
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            int index = r*3 + c;
            res += kernel[index] * test.at(index);
        }
    }
    std::cout << glm::to_string(res) << std::endl;

    glm::vec3 r1, r2, r3;
    r1 = 1.f/4.f * test.at(0) + 1.f/2.f * test.at(1) + 1.f/4.f * test.at(2);
    r2 = 1.f/4.f * test.at(3) + 1.f/2.f * test.at(4) + 1.f/4.f * test.at(5);
    r3 = 1.f/4.f * test.at(6) + 1.f/2.f * test.at(7) + 1.f/4.f * test.at(8);
    res = 1.f/4.f * r1 + 1.f/2.f * r2 + 1.f/4.f * r3;
    std::cout << glm::to_string(res) << std::endl;


    std::srand(std::time(0));

    QApplication app(argc, argv);
    MainWindow w;
    bool startFullscreen = false;

    w.show();

    if (startFullscreen) {
        // We cannot use w.showFullscreen() here because on Linux that creates the
        // window behind all other windows, so we have to set it to fullscreen after
        // it has been shown.
        w.setWindowState(w.windowState() | Qt::WindowFullScreen);
    }

    return app.exec();
}
