#include "frontend_manager.h"
#include <qhashfunctions.h>

FrontendManager::FrontendManager(PipeWireManager *pipewire_manager, KConfig *config, QObject *parent) : QObject(parent) {
    m_pipewire_manager = pipewire_manager;
    m_config = config;
    m_config_settings = m_config->group(QStringLiteral("Settings"));
    connect(m_pipewire_manager, &PipeWireManager::error_occured, this, &FrontendManager::set_error_message);

    // Initialize UI values

    load_hrir_wav_files();

    m_startup_ui = m_config_settings.readEntry("startup_ui", QString());
    // Fallback for malformed config value
    QStringList allowed_startup_ui_values = {QStringLiteral("showTray"), QStringLiteral("showTrayHideWindow"), QStringLiteral("hideTray")};
    if (!allowed_startup_ui_values.contains(m_startup_ui)) {
        m_startup_ui = QStringLiteral("hideTray"); // Hide by default because not all desktop environments support tray icons out of the box
        m_config_settings.writeEntry("startup_ui", m_startup_ui);
        m_config->sync();
    }

    m_autostart_enabled = m_config_settings.readEntry("autostart_enabled", bool());

    m_virtual_surround_auto_enabled = m_config_settings.readEntry("virtual_surround_auto_enabled", bool());
    if (m_virtual_surround_auto_enabled) {
        // This line gets intentionally skipped if auto-enabling is disabled
        set_virtual_surround_enabled(true);
    }
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
        if (m_pipewire_manager->create_virtual_surround_module(path.toStdString()))
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
    Q_EMIT error_message_changed();
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
    m_config_settings.writeEntry("hrir_wav_file_path", m_hrir_wav_file_paths.value(m_hrir_wav_file_name_index));
    m_config->sync();

    Q_EMIT hrir_wav_file_name_index_changed();
}

void FrontendManager::load_hrir_wav_files() {
    // Check first if a path has already been selected by the user
    QString old_path = m_hrir_wav_file_paths.value(m_hrir_wav_file_name_index);

    // Read from config file if nothing has been selected in current session
    if (old_path.isEmpty()) {
        old_path = m_config_settings.readEntry("hrir_wav_file_path", QString());
    }

    // Get all data directories, ordered by priority, first "~/.local/share/virtual-surround-manager", then "/usr/share/virtual-surround-manager"
    // In Flatpak: "~/.var/app/de.berny23.virtual_surround_manager/data/virtual-surround-manager", then (the PipeWire-unusable) "/app/share/virtual-surround-manager"
    QStringList data_dirs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

#ifdef IS_FLATPAK
    QDir internal_path(QStringLiteral("/app/share/virtual-surround-manager") + hrir_wav_subpath);
#endif

#ifdef IS_APPIMAGE
    // Read path from data_dirs variable because files are in a random folder inside /tmp
    QDir internal_path(data_dirs.value(1) + hrir_wav_subpath);
#endif

#if defined(IS_FLATPAK) || defined(IS_APPIMAGE)
    // PipeWire requires an absolut path on the host for loading filter chains!
    // The user-writable path is OK, since Qt already returns the full path, e. g. "/home/berny23/.var/app/de.berny23.virtual_surround_manager/data/virtual-surround-manager"
    // However, internal AppImage or Flatpak data folders like "/app/share/virtual-surround-manager" either cannot be used here at all or are temporary.
    // Because a Flatpak can be installed either in system or user mode, we don't know the exact host path. AppImages generally use temporary folders.
    // So we copy the files to the user-writable folder instead, without overwriting.
    if (internal_path.exists()) {
        QString writable_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + hrir_wav_subpath;

        QDir dir(writable_path);
        if (!dir.exists())
            dir.mkpath(QStringLiteral("."));

        internal_path.setNameFilters(QStringList({QStringLiteral("*.wav")}));
        for (const QFileInfo &file : internal_path.entryInfoList(QDir::Files)) {
            if (QFile(writable_path + QStringLiteral("/") + file.fileName()).exists())
                continue;

            QFile::copy(file.absoluteFilePath(), writable_path + QStringLiteral("/") + file.fileName());
        }
        qDebug("load_hrir_wav_files: Copied files from '%s' to '%s'", internal_path.absolutePath().toStdString().c_str(), writable_path.toStdString().c_str());
    }
#endif

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

void FrontendManager::open_hrir_wav_folder() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + hrir_wav_subpath;

    // Create user data path if not exists
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

QString FrontendManager::get_startup_ui() const {
    return m_startup_ui;
}

void FrontendManager::set_startup_ui(const QString &value) {
    m_startup_ui = value;

    // Write to config file
    m_config_settings.writeEntry("startup_ui", m_startup_ui);
    m_config->sync();

    Q_EMIT startup_ui_changed();
}

bool FrontendManager::get_autostart_enabled() {
    return m_autostart_enabled;
}

void FrontendManager::set_autostart_enabled(bool value) {
    if (m_autostart_enabled == value)
        return;
    m_autostart_enabled = value;

    // Get source and destination path for desktop file (only used for native installation and AppImage)
    // Source can either be /usr/share/applications or ~/.local/share/applications, when installed natively
    QString source = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).value(2) + QStringLiteral("/de.berny23.virtual_surround_manager.desktop");
    if (!QFile::exists(source))
        source = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).value(0) + QStringLiteral("/de.berny23.virtual_surround_manager.desktop");
    const QString destination = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).value(0) + QStringLiteral("/autostart/de.berny23.virtual_surround_manager.desktop");

#ifdef IS_FLATPAK
    // Use libportal to request Flatpak autostart
    g_autoptr(GPtrArray) cmd = nullptr;
    XdpBackgroundFlags flags = value ? XDP_BACKGROUND_FLAG_AUTOSTART : XDP_BACKGROUND_FLAG_NONE;
    xdp_portal_request_background(XdpQt::globalPortalObject(), nullptr, const_cast<char *>("Autostart"), cmd, flags, nullptr, nullptr, this);
#endif

    if (m_autostart_enabled) {
#ifdef IS_APPIMAGE
        // Copy desktop file from extraced AppImage folder that is automatically created under /tmp and randomly named. The APPDIR variable contains the path.
        source = qEnvironmentVariable("APPDIR") + QStringLiteral("/de.berny23.virtual_surround_manager.desktop");
#endif

#if !defined(IS_FLATPAK)
        // Native or AppImage: Just copy the desktop file to the user's autostart folder (e. g. ~/.config/autostart)
        // Creates directory with all parents, mkpath skips existing folders
        QDir().mkpath(QFileInfo(destination).path());
        QFile::copy(source, destination);
        qDebug("set_autostart_enabled: Copied desktop file from '%s' to '%s'", source.toStdString().c_str(), destination.toStdString().c_str());
#endif
#ifdef IS_APPIMAGE
        // Replace "Exec=virtual-surround-manager" with full path to AppImage
        QFile file(destination);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QString content = QTextStream(&file).readAll();
            content.replace(QStringLiteral("Exec=virtual-surround-manager"), QStringLiteral("Exec=") + qEnvironmentVariable("APPIMAGE"));
            file.seek(0);
            file.resize(0);
            QTextStream(&file) << content;
        }
#endif
    } else {
#if !defined(IS_FLATPAK)
        // Native or AppImage: Remove the desktop file from the user's autostart folder (e. g. ~/.config/autostart)
        QFile::remove(destination);
#endif
    }

    // Write to config file
    m_config_settings.writeEntry("autostart_enabled", m_autostart_enabled);
    m_config->sync();

    Q_EMIT autostart_enabled_changed();
}

bool FrontendManager::get_virtual_surround_auto_enabled() {
    return m_virtual_surround_auto_enabled;
}

void FrontendManager::set_virtual_surround_auto_enabled(bool value) {
    if (m_virtual_surround_auto_enabled == value)
        return;
    m_virtual_surround_auto_enabled = value;

    // Write to config file
    m_config_settings.writeEntry("virtual_surround_auto_enabled", m_virtual_surround_auto_enabled);
    m_config->sync();

    Q_EMIT virtual_surround_auto_enabled_changed();
}