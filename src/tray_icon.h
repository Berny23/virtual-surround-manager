#pragma once

#include "frontend_manager.h"

#include <KLocalizedString>
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QQuickWindow>
#include <QSystemTrayIcon>

class TrayIcon : public QObject {
    Q_OBJECT

  public:
    explicit TrayIcon(QObject *parent = nullptr);

    void set_frontend_manager(FrontendManager *frontend_manager);
    void setup(const QString &icon_name);
    void set_main_window(QQuickWindow *window);

    QSystemTrayIcon *get_tray_icon() const { return m_tray_icon; }

    Q_INVOKABLE void hide_main_window();
    Q_INVOKABLE void show_main_window();
    Q_INVOKABLE void quit_application();
    Q_INVOKABLE void toggle_virtual_surround();
    Q_INVOKABLE void update_toggle_action_state();

  private:
    void update_profile_menu();
    void on_profile_selected();
    void update_ui_state();

    QSystemTrayIcon *m_tray_icon = nullptr;
    QMenu *m_menu = nullptr;
    QMenu *m_profile_menu = nullptr;
    QAction *m_toggle_action = nullptr;
    QAction *m_last_selected_profile = nullptr;

    FrontendManager *m_frontend_manager = nullptr;
    QQuickWindow *m_main_window = nullptr;
};