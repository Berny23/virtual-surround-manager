#include "frontend_manager.h"
#include <qtmetamacros.h>

FrontendManager::FrontendManager(PipeWireManager *pipewire_manager, QObject *parent) : QObject(parent) {
    m_pipewire_manager = pipewire_manager;
    connect(m_pipewire_manager, &PipeWireManager::error_occured, this, &FrontendManager::on_pipewire_error);
}

bool FrontendManager::get_virtual_surround_enabled() {
    return m_virtual_surround_enabled;
}

void FrontendManager::set_virtual_surround_enabled(bool value) {
    if (m_virtual_surround_enabled == value)
        return;
    m_virtual_surround_enabled = value;

    if (m_virtual_surround_enabled) {
        m_pipewire_manager->create_virtual_surround_module();
        m_pipewire_manager->enable_routing();
    } else {
        m_pipewire_manager->disable_routing();
        // m_pipeWireManager->remove_virtual_surround_module();
    }

    Q_EMIT virtual_surround_enabled_changed();
}

QString FrontendManager::get_error_message() const {
    return m_error_message;
}

QStringList FrontendManager::get_hrir_wav_file_names() const {
    return m_hrir_wav_file_names;
}

int FrontendManager::get_hrir_wav_file_name_index() const {
    return m_hrir_wav_file_name_index;
}

void FrontendManager::set_hrir_wav_file_name_index(int index) {
    // Check if the index is valid
    if (index < 0 || index >= m_hrir_wav_file_names.size())
        return;
    if (m_hrir_wav_file_name_index == index)
        return;

    m_hrir_wav_file_name_index = index;

    m_pipewire_manager->create_virtual_surround_module(); // TODO: Supply different path

    Q_EMIT hrir_wav_file_name_index_changed();
}

void FrontendManager::on_pipewire_error(const QString &message) {
    m_error_message = message;
    Q_EMIT error_message_changed(); // TODO: Replace with thing that actually works
}
