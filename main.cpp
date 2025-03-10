#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include "barcodereader.h"
#include "barcodewriter.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    qmlRegisterType<ZXingQt::BarcodeReader>("ZXing", 1, 0, "BarcodeReader");
    qmlRegisterType<BarcodeWriter>("WriteBarcode", 1, 0, "WriteBarcode");


    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/BarcodeReader4/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
