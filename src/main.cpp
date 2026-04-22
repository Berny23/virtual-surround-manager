#include "frontend_manager.h"
#include "pipewire_manager.h"
#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QtQml>
#include <qhashfunctions.h>

int main(int argc, char *argv[]) {
    KIconTheme::initTheme();
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("virtual-surround-manager");
    QApplication::setOrganizationDomain(QStringLiteral("berny23.de"));
    QApplication::setApplicationName(QStringLiteral("virtual-surround-manager"));
    QApplication::setDesktopFileName(QStringLiteral("de.berny23.virtual-surround-manager"));

    QApplication::setStyle(QStringLiteral("breeze"));
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QQmlApplicationEngine engine;
    PipeWireManager pipewire_manager;
    FrontendManager *frontend_manager = new FrontendManager(&pipewire_manager);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("frontendManager"), frontend_manager);
    engine.loadFromModule("de.berny23.virtual-surround-manager", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
