#include "frontend_manager.h"
#include <qhashfunctions.h>

FrontendManager::FrontendManager(PipeWireManager *pipewire_manager, QObject *parent) : QObject(parent) {
    m_pipewire_manager = pipewire_manager;
    connect(m_pipewire_manager, &PipeWireManager::error_occured, this, &FrontendManager::set_error_message);
    load_hrir_wav_files();
}

bool FrontendManager::does_hrir_wav_file_exist(const QString &file_path) {
    if (!QFile(file_path).exists()) {
        qWarning("does_hrir_wav_file_exist: HRIR file does not exist: %s", file_path.toStdString().c_str());
        set_error_message(QStringLiteral("Selected HRIR file does not exist in your filesystem."));
        return false;
    }
    return true;
}

bool FrontendManager::get_virtual_surround_enabled() {
    return m_virtual_surround_enabled;
}

void FrontendManager::set_virtual_surround_enabled(bool value) {
    if (m_virtual_surround_enabled == value)
        return;
    m_virtual_surround_enabled = value;

    if (m_virtual_surround_enabled) {
        QString path = m_hrir_wav_file_paths.value(m_hrir_wav_file_name_index);
        if (!does_hrir_wav_file_exist(path))
            return;
        m_pipewire_manager->create_virtual_surround_module(path.toStdString());
        m_pipewire_manager->enable_routing();
    } else {
        m_pipewire_manager->disable_routing();
    }

    Q_EMIT virtual_surround_enabled_changed();
}

QString FrontendManager::get_error_message() const {
    return m_error_message;
}

void FrontendManager::set_error_message(const QString &message) {
    m_error_message = message;
    Q_EMIT error_message_changed(); // TODO: Replace with thing that actually works
}

QStringList FrontendManager::get_hrir_wav_file_names() const {
    return m_hrir_wav_file_names;
}

int FrontendManager::get_hrir_wav_file_name_index() const {
    return m_hrir_wav_file_name_index;
}

void FrontendManager::set_hrir_wav_file_name_index(int index) {
    qDebug("set_hrir_wav_file_name_index: %u", index);

    // Check if the index is valid
    if (index < 0 || index >= m_hrir_wav_file_names.size())
        return;
    if (m_hrir_wav_file_name_index == index)
        return;

    m_hrir_wav_file_name_index = index;

    // First, disable routing to prevent disruption of playing audio if the virtual surround sound is enabled
    if (m_virtual_surround_enabled)
        m_pipewire_manager->disable_routing();

    // We need to wait a little bit or the audio just stops completely when destroying the module too fast
    QThread::msleep(100);
    m_pipewire_manager->remove_virtual_surround_module();

    QString path = m_hrir_wav_file_paths.value(m_hrir_wav_file_name_index);
    if (!does_hrir_wav_file_exist(path))
        return;
    m_pipewire_manager->create_virtual_surround_module(path.toStdString());

    // At last, enable routing again if the virtual surround sound is enabled
    if (m_virtual_surround_enabled)
        m_pipewire_manager->enable_routing();

    // Write to config file
    KConfig config(QStringLiteral("virtual-surround-manager"));
    KConfigGroup group = config.group(QStringLiteral("Settings"));
    group.writeEntry("hrir_wav_file_path", m_hrir_wav_file_paths.value(m_hrir_wav_file_name_index));
    config.sync();

    Q_EMIT hrir_wav_file_name_index_changed();
}

void FrontendManager::load_hrir_wav_files() {
    // Check first if a path has already been selected by the user
    QString old_path = m_hrir_wav_file_paths.value(m_hrir_wav_file_name_index);

    // Read from config file if nothing has been selected in current session
    if (old_path.isEmpty()) {
        KConfig config(QStringLiteral("virtual-surround-manager"));
        KConfigGroup group = config.group(QStringLiteral("Settings"));
        old_path = group.readEntry("hrir_wav_file_path", QString());
    }

    // Get all data directories, ordered by priority, first "~/.local/share/virtual-surround-manager/", then "/usr/share/virtual-surround-manager/"
    QStringList data_dirs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    QStringList names;
    QStringList paths;

    for (const QString &path : data_dirs) {
        QDir dir(path + hrir_wav_subpath);
        if (!dir.exists())
            continue;

        // Get all .wav files in the directory, skip duplicates because the priority is user-first
        dir.setNameFilters(QStringList({QStringLiteral("*.wav")}));
        for (const QFileInfo &file : dir.entryInfoList(QDir::Files)) {
            if (names.contains(file.fileName()))
                continue;

            names.append(file.fileName());
            paths.append(file.absoluteFilePath());
        }
    }

    m_hrir_wav_file_names = names;
    m_hrir_wav_file_paths = paths;

    Q_EMIT hrir_wav_file_names_changed();

    // Try to restore the old file selection if it still exists
    if (!old_path.isEmpty()) {
        const int new_index = m_hrir_wav_file_paths.indexOf(old_path);
        if (new_index >= 0 && new_index != m_hrir_wav_file_name_index) {
            m_hrir_wav_file_name_index = new_index;
            Q_EMIT hrir_wav_file_name_index_changed();
            qDebug("Restored previous file selection");
        }
    } else {
        // Set to first entry if no selection has been made
        m_hrir_wav_file_name_index = 0;
    }
}

void FrontendManager::openHrirWavFolder() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + hrir_wav_subpath;

    // Create user data path if not exists
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
