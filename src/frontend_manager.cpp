#include "frontend_manager.h"

FrontendManager::FrontendManager(PipeWireManager *pipeWireManager, QObject *parent) : QObject(parent) {
    m_pipeWireManager = pipeWireManager;
    connect(m_pipeWireManager, &PipeWireManager::errorOccured, this, &FrontendManager::onPipeWireError);
}

bool FrontendManager::getVirtualSurroundEnabled() {
    return m_virtualSurroundEnabled;
}

void FrontendManager::setVirtualSurroundEnabled(bool value) {
    if (m_virtualSurroundEnabled == value)
        return;
    m_virtualSurroundEnabled = value;

    if (m_virtualSurroundEnabled) {
        m_pipeWireManager->create_virtual_surround_module();
        m_pipeWireManager->enable_routing();
    } else {
        m_pipeWireManager->disable_routing();
        //m_pipeWireManager->remove_virtual_surround_module();
    }

    Q_EMIT virtualSurroundEnabledChanged();
}

void FrontendManager::onPipeWireError(const QString &message) {
    m_errorMessage = message;
    Q_EMIT errorMessageChanged();
}

QString FrontendManager::getErrorMessage() const {
    return m_errorMessage;
}