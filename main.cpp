#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "filereader.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    FileReader fileReader;
    engine.rootContext()->setContextProperty("FileReader", &fileReader);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("SimpleFileViewer", "Main");

    return app.exec();
}
