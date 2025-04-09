#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include "autotrackercontroller.h"
#include <NvInferVersion.h>
// Add debug functionality for tracking object creation
void debugObjectCreation(const QString& className, QObject* obj) {
    if (obj) {
        qDebug() << "Created" << className << "at" << obj;
    } else {
        qDebug() << "FAILED to create" << className << "- got nullptr";
    }
}

int main(int argc, char *argv[])
{
    qDebug() << "TensorRT" << NV_TENSORRT_MAJOR << "." << NV_TENSORRT_MINOR << "." << NV_TENSORRT_PATCH;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("comcept");
    app.setOrganizationDomain("autotracker-v7");

    // Install message handler for better debugging
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QString txt;
        switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtInfoMsg:
            txt = QString("Info: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            break;
        }
        // Output to console
        fprintf(stderr, "%s\n", qPrintable(txt));
    });

    QQmlApplicationEngine engine;

    qRegisterMetaType<PacketData>("PacketData");
    qRegisterMetaType<TrackerFrame>("TrackerFrame");
    qRegisterMetaType<ObjectState>("ObjectState");
    qRegisterMetaType<ObjectMotion>("ObjectMotion");
    qRegisterMetaType<OPResult>("OPResult");
    qRegisterMetaType<QVector<QRect>>("QVector<QRect>");
    qRegisterMetaType<QVector<QRectF>>("QVector<QRectF>");
    qRegisterMetaType<QVector<QVector<QVector2D>>>("QVector<QVector<QVector2D>>");
    qRegisterMetaType<QVector<QPointF>>("QVector<QPointF>");
    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QVector<QVector<int>>>("QVector<QVector<int>>");
    qRegisterMetaType<QVector <YoloResult>>("QVector <YoloResult>");

    qmlRegisterType<AutoTrackerController>("AutoTrackerControllerClass", 1, 0, "AutoTrackerController");
    qmlRegisterUncreatableMetaObject(DATAFIELDS::staticMetaObject, "DataFields", 1, 0, "DATA_FIELDS", "Error: only enums");

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qDebug() << "Failed to create QML root object";
                QCoreApplication::exit(-1);
            } else if (obj) {
                qDebug() << "Successfully created QML root object";
            }
        }, Qt::QueuedConnection);

    qDebug() << "Loading QML file...";
    engine.load(url);
    qDebug() << "QML file loaded";

    return app.exec();
}
