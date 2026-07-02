#pragma once

#include "pipewire_manager.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDesktopServices>
#include <QObject>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qdir.h>
#include <qhashfunctions.h>
#include <qlogging.h>
#include <qstandardpaths.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <qurl.h>

#ifdef IS_FLATPAK
#include <glib.h>
#include <glibconfig.h>
#include <libportal-qt6/portal-qt6.h>
#include <libportal/background.h>
#endif

class FrontendManager : public QObject {
    Q_OBJECT
    // Whether virtual surround sound is enabled or not
    Q_PROPERTY(bool virtualSurroundEnabled READ get_virtual_surround_enabled WRITE set_virtual_surround_enabled NOTIFY virtual_surround_enabled_changed)
    // The error message to show in the UI
    Q_PROPERTY(QString errorMessage READ get_error_message WRITE set_error_message NOTIFY error_message_changed)
    // The list of HRIR WAV file names found in the directory
    Q_PROPERTY(QStringList hrirWavFileNames READ get_hrir_wav_file_names NOTIFY hrir_wav_file_names_changed)
    // The user-selected index of the HRIR WAV file list
    Q_PROPERTY(int hrirWavFileNameIndex READ get_hrir_wav_file_name_index WRITE set_hrir_wav_file_name_index NOTIFY hrir_wav_file_name_index_changed)

    // The selected startup setting
    Q_PROPERTY(QString startupUi READ get_startup_ui WRITE set_startup_ui NOTIFY startup_ui_changed)
    // Whether autostart is enabled or not
    Q_PROPERTY(bool autostartEnabled READ get_autostart_enabled WRITE set_autostart_enabled NOTIFY autostart_enabled_changed)
    // Whether virtual surround sound is enabled automatically or not
    Q_PROPERTY(bool virtualSurroundAutoEnabled READ get_virtual_surround_auto_enabled WRITE set_virtual_surround_auto_enabled NOTIFY virtual_surround_auto_enabled_changed)

  public:
    explicit FrontendManager(PipeWireManager *pipewire_manager, KConfig *config, QObject *parent = nullptr);

    bool get_virtual_surround_enabled();
    void set_virtual_surround_enabled(bool value);
    Q_SIGNAL void virtual_surround_enabled_changed();

    QString get_error_message() const;
    void set_error_message(const QString &value);
    Q_SIGNAL void error_message_changed();

    QStringList get_hrir_wav_file_names() const;
    Q_SIGNAL void hrir_wav_file_names_changed();

    int get_hrir_wav_file_name_index() const;
    void set_hrir_wav_file_name_index(int index);
    Q_SIGNAL void hrir_wav_file_name_index_changed();

    // Load HRIR WAV files from data directory
    Q_INVOKABLE void load_hrir_wav_files();

    // Open folder in default file explorer
    Q_INVOKABLE void open_hrir_wav_folder();

    QString get_startup_ui() const;
    void set_startup_ui(const QString &value);
    Q_SIGNAL void startup_ui_changed();

    bool get_autostart_enabled();
    void set_autostart_enabled(bool value);
    Q_SIGNAL void autostart_enabled_changed();

    bool get_virtual_surround_auto_enabled();
    void set_virtual_surround_auto_enabled(bool value);
    Q_SIGNAL void virtual_surround_auto_enabled_changed();

  private:
    bool m_virtual_surround_enabled = false;
    QString m_error_message;
    QStringList m_hrir_wav_file_names;
    QStringList m_hrir_wav_file_paths;
    int m_hrir_wav_file_name_index = 0;
    QString m_startup_ui;
    bool m_autostart_enabled = false;
    bool m_virtual_surround_auto_enabled = false;

    PipeWireManager *m_pipewire_manager;
    KConfig *m_config;
    KConfigGroup m_config_settings;

    QString hrir_wav_subpath = QStringLiteral("/hrir_wav");

    bool does_hrir_wav_file_exist(const QString &file_path);
};