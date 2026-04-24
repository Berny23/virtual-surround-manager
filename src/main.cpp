#include "frontend_manager.h"
#include "pipewire_manager.h"
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QtQml>
#include <kaboutdata.h>
#include <qapplication.h>
#include <qhashfunctions.h>
#include <qjsengine.h>
#include <qjsvalue.h>
#include <qqml.h>
#include <qqmlengine.h>
#include <qstringview.h>

//
// This checks for an already running application.
// Adapted from EasyEffects: https://github.com/wwmm/easyeffects/blob/master/src/util.cpp
//
static std::unique_ptr<QLockFile> get_lock_file() {
    auto sys_path = std::filesystem::exists("/.flatpak-info") ? QStandardPaths::TempLocation : QStandardPaths::RuntimeLocation;

    auto lock_file = std::make_unique<QLockFile>(QString::fromStdString(QStandardPaths::writableLocation(sys_path).toStdString() + "/virtual-surround-manager.lock"));

    lock_file->setStaleLockTime(0);

    bool status = lock_file->tryLock(100);

    if (!status) {
        qDebug("get_lock_file: Could not lock the file: %s", lock_file->fileName().toStdString().c_str());

        switch (lock_file->error()) {
        case QLockFile::NoError:
            break;
        case QLockFile::LockFailedError: {
            qDebug("get_lock_file: Another instance already has the lock");
            break;
        }
        case QLockFile::PermissionError: {
            qDebug("get_lock_file: No permission to create the lock file");
            break;
        }
        case QLockFile::UnknownError: {
            qDebug("get_lock_file: Unknown error");
            break;
        }
        }
    }

    return lock_file;
}

int main(int argc, char *argv[]) {
    // Initialize GUI
    KIconTheme::initTheme();
    QApplication app(argc, argv);

    // App information
    KLocalizedString::setApplicationDomain("virtual-surround-manager");
    QApplication::setOrganizationDomain(QStringLiteral("berny23.de"));
    QApplication::setApplicationName(QStringLiteral("virtual-surround-manager"));
    QApplication::setDesktopFileName(QStringLiteral("de.berny23.virtual_surround_manager"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("de.berny23.virtual_surround_manager")));

    // Set theme
    QApplication::setStyle(QStringLiteral("breeze"));
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    // Credits
    KAboutData aboutData(QStringLiteral("virtual-surround-manager"),
                         i18nc("@title", "Virtual Surround Sound"),
                         QStringLiteral("1.0"),
                         i18nc("@title:window", "Virtual Surround Sound Manager"),
                         KAboutLicense::MIT,
                         QStringLiteral("© 2026, Berny23"));
    aboutData.setOrganizationDomain("berny23.de");
    aboutData.setDesktopFileName(QStringLiteral("de.berny23.virtual_surround_manager"));
    aboutData.setProgramLogo(QStringLiteral("de.berny23.virtual_surround_manager"));
    aboutData.setBugAddress("https://github.com/Berny23/virtual-surround-manager/issues");
    aboutData.addComponent(KAboutComponent(
        i18nc("@component", "Contains HRIR WAV files from HeSuVi. © 2021, Matt Gore. See: hesuvi_license.txt"),
        QStringLiteral(""),
        QStringLiteral(""),
        QStringLiteral("https://hesuvi.net"),
        KAboutLicense::MIT));
    aboutData.addAuthor(
        QStringLiteral("Berny23"),
        i18nc("@info:credit", "Developer"),
        QStringLiteral(""),
        QStringLiteral("https://berny23.de"));
    KAboutData::setApplicationData(aboutData);

    // Add about page
    qmlRegisterSingletonType("de.berny23.virtual_surround_manager",
                             1,
                             0,
                             "About",
                             [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
                                 return engine->toScriptValue(KAboutData::applicationData());
                             });

    // Force single instance
    auto lockFile = get_lock_file();
    if (!lockFile->isLocked()) {
        qDebug("Another instance of this application is already running");
        return 0;
    }

    // Create PipeWire and frontend manager
    PipeWireManager pipewire_manager;
    FrontendManager *frontend_manager = new FrontendManager(&pipewire_manager);

    // Add main page and supply context
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("frontendManager"), frontend_manager);
    engine.loadFromModule("de.berny23.virtual_surround_manager", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    // Show GUI
    return app.exec();
}
