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

    width: 400
    height: 300
    title: i18nc("@title:window", "Virtual Surround Manager")

    pageStack.initialPage: Kirigami.Page {
        id: mainPage
        actions: [
            // Global toggle for enabling or disabling the virtual surround routing
            Kirigami.Action {
                id: toggleVirtualSurroundAction
                text: i18nc("@label", "Toggle Virtual Surround")
                checkable: true
                checked: settings.virtualSurroundEnabled // TODO: Save state and reapply on start
                onToggled: {
                    root.toggleVirtualSurround(checked);
                }
                displayComponent: Controls.Switch {
                    action: toggleVirtualSurroundAction
                    onCheckedChanged: settings.virtualSurroundEnabled = checked
                }
            }
        ]
        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: settings.errorMessage
            visible: settings.errorMessage.length > 0
        }
    }

    // Functions
    function toggleVirtualSurround(value): void {
        settings.virtualSurroundEnabled = value;
        console.log("Toggle Virtual Surround: " + value);
    }
}
