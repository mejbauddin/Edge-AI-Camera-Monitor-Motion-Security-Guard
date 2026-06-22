#include "UiBootstrap.hpp"

#include "Application.hpp"
#include "ui/bridge/DashboardBridge.hpp"
#include "ui/bridge/EnrollmentController.hpp"
#include "ui/providers/FrameImageProvider.hpp"
#include "ui/providers/RadarImageProvider.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

#include <iostream>

namespace csx::ui {

namespace {

QString resolveQmlPath() {
    const QString besideExe = QCoreApplication::applicationDirPath() + QStringLiteral("/qml/main.qml");
    if (QFile::exists(besideExe)) {
        return besideExe;
    }

    const QString projectRelative = QStringLiteral("assets/qml/main.qml");
    if (QFile::exists(projectRelative)) {
        return QFileInfo(projectRelative).absoluteFilePath();
    }

    return besideExe;
}

}  // namespace

int runUi(Application& app, int argc, char* argv[]) {
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    QGuiApplication qtApp(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Cyber Sentinel X"));
    QCoreApplication::setOrganizationName(QStringLiteral("CyberSentinel"));
    qtApp.setQuitOnLastWindowClosed(true);

    auto* bridge = new DashboardBridge(&qtApp);
    auto* enrollment = new EnrollmentController(&app, &qtApp);
    auto* frameProvider = new FrameImageProvider();
    auto* radarProvider = new RadarImageProvider(app.radarModel());

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("frames"), frameProvider);
    engine.addImageProvider(QStringLiteral("radar"), radarProvider);
    engine.rootContext()->setContextProperty(QStringLiteral("dashboard"), bridge);
    engine.rootContext()->setContextProperty(QStringLiteral("enrollment"), enrollment);

    const auto qmlPath = resolveQmlPath();
    if (!QFile::exists(qmlPath)) {
        std::cerr << "QML not found: " << qmlPath.toStdString() << std::endl;
        return 1;
    }

    engine.load(QUrl::fromLocalFile(qmlPath));
    if (engine.rootObjects().isEmpty()) {
        std::cerr << "Failed to load QML: " << qmlPath.toStdString() << std::endl;
        return 1;
    }

    if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
        window->showMaximized();
    }

    QTimer bootTimer(&qtApp);
    QObject::connect(&bootTimer, &QTimer::timeout, [bridge]() {
        bridge->setBootComplete(true);
    });
    bootTimer.setSingleShot(true);
    bootTimer.start(4200);

    QTimer visionTimer(&qtApp);
    visionTimer.setInterval(33);
    QObject::connect(&visionTimer, &QTimer::timeout, [&app, bridge, frameProvider]() {
        VisionUiSnapshot snapshot;
        if (app.captureVisionSnapshot(snapshot)) {
            app.applyVisionSnapshot(bridge, frameProvider, snapshot);
        }
    });
    visionTimer.start();

    QObject::connect(&qtApp, &QGuiApplication::aboutToQuit, [&app]() {
        app.shutdown();
    });

    std::cout << "Cyber Sentinel X UI online" << std::endl;
    return qtApp.exec();
}

}  // namespace csx::ui
