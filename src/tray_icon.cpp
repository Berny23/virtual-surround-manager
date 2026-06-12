#include "tray_icon.h"
#include <qhashfunctions.h>

TrayIcon::TrayIcon(QObject *parent) : QObject(parent) {
    m_tray_icon = new QSystemTrayIcon(this);
    m_menu = new QMenu(nullptr);

    // Build context menu
    auto *open_action = m_menu->addAction(i18nc("@action", "Open"));
    m_toggle_action = m_menu->addAction(i18nc("@action:switch", "Enabled"));
    m_toggle_action->setCheckable(true);
    m_profile_menu = m_menu->addMenu(i18nc("@action", "Profiles"));
    m_menu->addSeparator();
    auto *quit_action = m_menu->addAction(i18nc("@action", "Quit"));

    // Connect actions
    QObject::connect(open_action, &QAction::triggered, this, &TrayIcon::show_main_window);
    QObject::connect(m_toggle_action, &QAction::triggered, this, &TrayIcon::toggle_virtual_surround);
    QObject::connect(quit_action, &QAction::triggered, this, &TrayIcon::quit_application);

    // Double-click / middle-click on tray icon shows main window
    QObject::connect(m_tray_icon, &QSystemTrayIcon::activated, this,
                     [this](QSystemTrayIcon::ActivationReason reason) {
                         if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::MiddleClick) {
                             show_main_window();
                         }
                     });
}

void TrayIcon::set_frontend_manager(FrontendManager *frontend_manager) {
    m_frontend_manager = frontend_manager;
    update_profile_menu();
}

void TrayIcon::setup(const QString &icon_name) {
    QIcon icon = QIcon::fromTheme(icon_name);
#ifdef IS_APPIMAGE
    if (icon.isNull()) {
        // The tray icon does NOT work automatically in an AppImage for unknown reasons!
        // Both fromTheme and addFile are not working, so the svg file just gets copied to ~/.local/share/icons/hicolor/scalable/apps/
        QString source = qEnvironmentVariable("APPDIR") + QStringLiteral("/") + icon_name + QStringLiteral(".svg");
        QString destination = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/icons/hicolor/scalable/apps/") + icon_name + QStringLiteral(".svg");
        if (!QFile::exists(destination)) {
            QFile::copy(source, destination);
            qDebug("TrayIcon::setup: Copied icon file from '%s' to '%s'.", source.toStdString().c_str(), destination.toStdString().c_str());
        }
        icon.addFile(destination);
    }
#endif
    m_tray_icon->setIcon(icon);
    m_tray_icon->setToolTip(i18nc("@title", "Virtual Surround Sound"));
    m_tray_icon->setContextMenu(m_menu);
}

void TrayIcon::set_main_window(QQuickWindow *window) {
    m_main_window = window;
}

void TrayIcon::set_visibility(bool visible) {
    if (visible) {
        m_tray_icon->show();
    } else {
        m_tray_icon->hide();
    }
}

bool TrayIcon::is_visible() {
    return m_tray_icon->isVisible();
}

void TrayIcon::hide_main_window() {
    if (m_main_window) {
        m_main_window->hide();
    }
}

void TrayIcon::show_main_window() {
    if (m_main_window) {
        m_main_window->showNormal();
        m_main_window->raise();
    } else {
        qWarning() << "TrayIcon::show_main_window - m_main_window is null!";
    }
}

void TrayIcon::quit_application() {
    m_tray_icon->hide();
    QApplication::quit();
}

void TrayIcon::toggle_virtual_surround() {
    if (!m_frontend_manager) {
        return;
    }

    const bool enabled = !m_frontend_manager->get_virtual_surround_enabled();
    m_frontend_manager->set_virtual_surround_enabled(enabled);
    update_ui_state();
}

void TrayIcon::update_toggle_action_state() {
    if (!m_frontend_manager) {
        return;
    }
    update_ui_state();
}

void TrayIcon::update_ui_state() {
    if (!m_frontend_manager || !m_toggle_action) {
        return;
    }

    const bool enabled = m_frontend_manager->get_virtual_surround_enabled();
    m_toggle_action->setChecked(enabled);
}

void TrayIcon::update_profile_menu() {
    if (m_called_update_profile_menu) {
        // Prevent calling twice and causing multible checkmarks.
        m_called_update_profile_menu = false;
        return;
    }

    if (!m_frontend_manager || !m_profile_menu) {
        qWarning() << "TrayIcon::update_profile_menu - missing manager or menu";
        return;
    }

    // Clear existing profile actions
    for (auto *action : m_profile_menu->actions()) {
        m_profile_menu->removeAction(action);
        QObject::disconnect(action);
        delete action;
    }

    const auto profiles = m_frontend_manager->get_hrir_wav_file_names();
    const int current_index = m_frontend_manager->get_hrir_wav_file_name_index();

    if (profiles.isEmpty()) {
        auto *no_profiles_action = m_profile_menu->addAction(i18nc("@label", "No profiles available"));
        no_profiles_action->setEnabled(false);
        return;
    }

    for (int i = 0; i < profiles.size(); ++i) {
        auto *action = m_profile_menu->addAction(profiles[i]);
        action->setCheckable(true);
        action->setData(i);
        action->setChecked(i == current_index);
        QObject::connect(action, &QAction::triggered, this, &TrayIcon::on_profile_selected);
    }
}

void TrayIcon::on_profile_selected() {
    if (!m_frontend_manager) {
        return;
    }

    auto *action = qobject_cast<QAction *>(sender());
    if (!action) {
        qWarning() << "TrayIcon::on_profile_selected - sender is not a QAction!";
        return;
    }

    m_called_update_profile_menu = true;

    const int index = action->data().toInt();
    m_frontend_manager->set_hrir_wav_file_name_index(index);

    for (auto *a : m_profile_menu->actions()) {
        a->setChecked(a == action);
    }
}