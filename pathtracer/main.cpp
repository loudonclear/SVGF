#include <QCoreApplication>
#include <QCommandLineParser>

#include <iostream>
#include <chrono>

#include "pathtracer.h"
#include "scene/scene.h"

#include <QImage>

#include "util/CS123Common.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("scene", "Scene file to be rendered");
    parser.addPositionalArgument("output", "Image file to write the rendered image to");
    parser.addPositionalArgument("numSamples", "Number of sampes per pixel");

    parser.process(a);

    const QStringList args = parser.positionalArguments();
    if(args.size() != 3) {
        cerr << "Error: Wrong number of arguments" << endl;
        a.exit(1);
        return 1;
    }
    QString scenefile = args[0];
    QString output = args[1];
    int numSamples = args[2].toInt();

    QImage image(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB32);

    Scene *scene;
    if(!Scene::load(scenefile, &scene)) {
        cerr << "Error parsing scene file " << scenefile.toStdString() << endl;
        a.exit(1);
        return 1;
    }

    PathTracer tracer(IMAGE_WIDTH, IMAGE_HEIGHT, numSamples);

    QRgb *data = reinterpret_cast<QRgb *>(image.bits());

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    tracer.traceScene(data, *scene);
    delete scene;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    auto duration = duration_cast<seconds>( t2 - t1 ).count();
    cout << "Scene took " << duration << " seconds to render." << endl;

    bool success = image.save(output);
    if(!success) {
        success = image.save(output, "PNG");
    }
    if(success) {
        cout << "Wrote rendered image to " << output.toStdString() << endl;
    } else {
        cerr << "Error: failed to write image to " << output.toStdString() << endl;
    }
    a.exit();
}
