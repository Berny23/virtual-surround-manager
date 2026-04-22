pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

// Provides basic features needed for all kirigami applications
Kirigami.ApplicationWindow {
    id: root
    width: 600
    height: 400
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
                //enabled: frontendManager.hrirFileNames.length > 0 // TODO: Implement
                onToggled: {
                    root.toggleVirtualSurround(checked);
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
                visible: true
                //visible: frontendManager.errorMessage.length > 0
                showCloseButton: true
            }

            Controls.ScrollView {
                Layout.fillHeight: true
                Kirigami.SearchField {
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onTextChanged: hrirWavFileSelection.filterText = text
                    KeyNavigation.tab: hrirWavFileSelection
                }
                // TODO: Fix this stuff not showing
                ListView {
                    id: hrirWavFileSelection
                    model: frontendManager.hrirWavFileNames
                    currentIndex: frontendManager.hrirWavFileNameIndex
                    delegate: Controls.ItemDelegate {
                        required property int index
                        required property string modelData

                        width: hrirWavFileSelection.width
                        text: modelData
                        highlighted: hrirWavFileSelection.currentIndex === index

                        onClicked: {
                            hrirWavFileSelection.currentIndex = index;
                            frontendManager.hrirWavFileNameIndex = index;
                        }
                    }
                }
            }

            // TODO: Replace example code with note about where to get more wav files. Also include note about EasyEffects
            Kirigami.InlineMessage {
                Layout.fillWidth: true
                text: "Check out <a href=\"https://kde.org\">KDE's website!<a/>"
                onLinkActivated: Qt.openUrlExternally(link)
                visible: true
            }
        }
    }

    // Functions
    function toggleVirtualSurround(value): void {
        frontendManager.virtualSurroundEnabled = value;
    }
}
