#pragma once
#include <QObject>
#include <qhashfunctions.h>
#include <qtmetamacros.h>
#include "pipewire_manager.h"

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool virtualSurroundEnabled READ getVirtualSurroundEnabled WRITE setVirtualSurroundEnabled NOTIFY virtualSurroundEnabledChanged)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessageChanged)

  public:
    explicit Settings(PipeWireManager *pipeWireManager, QObject *parent = NULL);

    bool getVirtualSurroundEnabled();
    void setVirtualSurroundEnabled(bool value);

    QString getErrorMessage() const;

  Q_SIGNALS:
    void virtualSurroundEnabledChanged();
    void errorMessageChanged();

  private Q_SLOTS:
    void onPipeWireError(const QString &message);

  private:
    bool m_virtualSurroundEnabled = false;
    QString m_errorMessage;
    PipeWireManager *m_pipeWireManager;
};