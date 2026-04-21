#pragma once

#include "pipewire_manager.h"
#include <QObject>
#include <qcontainerfwd.h>
#include <qhashfunctions.h>
#include <qtmetamacros.h>

class FrontendManager : public QObject {
    Q_OBJECT
    // Whether virtual surround sound is enabled or not
    Q_PROPERTY(bool virtualSurroundEnabled READ get_virtual_surround_enabled WRITE set_virtual_surround_enabled NOTIFY virtual_surround_enabled_changed)
    // The error message to show in the UI
    Q_PROPERTY(QString errorMessage READ get_error_message NOTIFY error_message_changed)
    // The list of HRIR WAV file names found in the directory
    Q_PROPERTY(QStringList hrirWavFileNames READ get_hrir_wav_file_names NOTIFY hrir_wav_file_names_changed)
    // The user-selected index of the HRIR WAV file list
    Q_PROPERTY(int hrirWavFileNameIndex READ get_hrir_wav_file_name_index WRITE set_hrir_wav_file_name_index NOTIFY hrir_wav_file_name_index_changed)

  public:
    explicit FrontendManager(PipeWireManager *pipewire_manager, QObject *parent = nullptr);

    bool get_virtual_surround_enabled();
    void set_virtual_surround_enabled(bool value);
    Q_SIGNAL void virtual_surround_enabled_changed();

    QString get_error_message() const;
    Q_SIGNAL void error_message_changed();

    QStringList get_hrir_wav_file_names() const;
    Q_SIGNAL void hrir_wav_file_names_changed();

    int get_hrir_wav_file_name_index() const;
    void set_hrir_wav_file_name_index(int index);
    Q_SIGNAL void hrir_wav_file_name_index_changed();

  private Q_SLOTS:
    void on_pipewire_error(const QString &message); // TODO: Fix error messaging via slot and emit

  private:
    bool m_virtual_surround_enabled = false;
    QString m_error_message;
    QStringList m_hrir_wav_file_names;
    int m_hrir_wav_file_name_index = 0;
    PipeWireManager *m_pipewire_manager;
};