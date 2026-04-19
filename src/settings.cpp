#include "settings.h"
#include "pipewire_manager.h"

Settings::Settings(PipeWireManager *pipeWireManager, QObject *parent) : QObject(parent) {
    m_pipeWireManager = pipeWireManager;
    connect(m_pipeWireManager, &PipeWireManager::errorOccured, this, &Settings::onPipeWireError);
}

bool Settings::getVirtualSurroundEnabled() {
    return m_virtualSurroundEnabled;
}

void Settings::setVirtualSurroundEnabled(bool value) {
    if (m_virtualSurroundEnabled == value)
        return;
    m_virtualSurroundEnabled = value;

    // TODO: Implement logic to enable or disable the virtual surround routing

    Q_EMIT virtualSurroundEnabledChanged();
}

void Settings::onPipeWireError(const QString &message) {
    m_errorMessage = message;
    Q_EMIT errorMessageChanged();
}

QString Settings::getErrorMessage() const {
    return m_errorMessage;
}