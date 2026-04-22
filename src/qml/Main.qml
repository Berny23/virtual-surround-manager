pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KirigamiDelegates

// Provides basic features needed for all kirigami applications
Kirigami.ApplicationWindow {
    id: root
    width: 550
    height: 550
    title: i18nc("@title:window", "Virtual Surround Sound Manager")

    pageStack.initialPage: Kirigami.Page {
        id: mainPage
        title: i18nc("@page:title", "Options")
        actions: [
            // Global toggle for enabling or disabling the virtual surround routing
            Kirigami.Action {
                id: toggleVirtualSurroundAction
                text: i18nc("@action:switch", "Enabled")
                checkable: true
                checked: frontendManager.virtualSurroundEnabled
                enabled: frontendManager.hrirWavFileNames.length > 0
                onToggled: {
                    frontendManager.virtualSurroundEnabled = checked;
                }
                displayComponent: Controls.Switch {
                    action: toggleVirtualSurroundAction
                }
            }
        ]

        ColumnLayout {
            anchors.fill: parent

            // This shows error messages, normally hidden
            Kirigami.InlineMessage {
                id: errorMessage
                Layout.fillWidth: true
                type: Kirigami.MessageType.Error
                text: i18nc("@label", "An error occured: %1", frontendManager.errorMessage)
                visible: frontendManager.errorMessage.length > 0
                showCloseButton: true
            }

            // Just used to group the radio buttons logically
            Controls.ButtonGroup {
                id: hrirWavFileNamesGroup
                onCheckedButtonChanged: {
                    frontendManager.hrirWavFileNameIndex = buttons.indexOf(checkedButton);
                }
            }

            Kirigami.Card {
                actions: [
                    Kirigami.Action {
                        text: i18nc("@action", "Open folder")
                        icon.name: "folder-open"
                        onTriggered: {
                            frontendManager.openHrirWavFolder();
                        }
                    },
                    Kirigami.Action {
                        text: i18nc("@action", "Reload")
                        icon.name: "object-rotate-right"
                        onTriggered: {
                            frontendManager.load_hrir_wav_files();
                        }
                    }
                ]
                Layout.fillHeight: true
                Layout.fillWidth: true
                header: Kirigami.Heading {
                    text: i18nc("@header", "Virtualization Profiles")
                    level: 2
                }
                contentItem: ListView {
                    model: frontendManager.hrirWavFileNames
                    delegate: Controls.RadioDelegate {
                        required property string modelData
                        required property int index

                        width: parent.width
                        text: modelData
                        checked: index == frontendManager.hrirWavFileNameIndex
                        Controls.ButtonGroup.group: hrirWavFileNamesGroup
                    }
                }
            }

            // TODO: Replace example code with note about where to get more wav files. Also include note about EasyEffects
            Kirigami.InlineMessage {
                Layout.fillWidth: true
                text: i18nc("@message", "<b>Adding more profiles</b><br>Just open the folder and place your WAV files there. You can find hundreds of these files in the <a href=\"https://airtable.com/appayGNkn3nSuXkaz/shruimhjdSakUPg2m/tbloLjoZKWJDnLtTc\">HRTF Database<a/>.")
                onLinkActivated: function (link) {
                    Qt.openUrlExternally(link);
                }
                visible: true
            }
            Kirigami.InlineMessage {
                Layout.fillWidth: true
                text: i18nc("@message", "<b>Using with EasyEffects</b><br>Exclude applications in EasyEffects that use 7.1 surround sound. Apply your effects on this programm instead.")
                onLinkActivated: function (link) {
                    Qt.openUrlExternally(link);
                }
                visible: true
            }
        }
    }
}
