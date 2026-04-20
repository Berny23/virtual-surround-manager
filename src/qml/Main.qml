// Includes relevant modules used by the QML

// Prevents:
// Unqualified access: Set "pragma ComponentBehavior: Bound" in order to use IDs from outer components in nested components.
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
    title: i18nc("@title:window", "Virtual Surround Manager")

    pageStack.initialPage: Kirigami.Page {
        id: mainPage
        title: i18nc("@page:title", "Options")
        actions: [
            // Global toggle for enabling or disabling the virtual surround routing
            Kirigami.Action {
                id: toggleVirtualSurroundAction
                text: i18nc("@action:switch", "Toggle Virtual Surround")
                checkable: true
                checked: frontendManager.virtualSurroundEnabled
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
            Kirigami.InlineMessage {
                id: errorMessage
                Layout.fillWidth: true
                type: Kirigami.MessageType.Error
                text: i18nc("@label:errorMessage", "An error occured: %1", frontendManager.errorMessage)
                visible: frontendManager.errorMessage.length > 0
                showCloseButton: true
            }
        }
    }

    // Functions
    function toggleVirtualSurround(value): void {
        frontendManager.virtualSurroundEnabled = value;
    }
}
