pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root
    title: i18nc("@title:window", "Settings")

    FormCard.FormHeader {
        title: i18nc("@title:group", "General")
    }

    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            id: autostartEnabled
            text: i18nc("@label:switch", "Automatically start application on login")
            checkable: true
            checked: frontendManager.autostartEnabled
            onToggled: frontendManager.autostartEnabled = checked
        }

        FormCard.FormSwitchDelegate {
            id: virtualSurroundAutoEnabled
            text: i18nc("@label:switch", "Automatically enable virtual surround sound")
            checkable: true
            checked: frontendManager.virtualSurroundAutoEnabled
            onToggled: frontendManager.virtualSurroundAutoEnabled = checked
        }

        FormCard.FormComboBoxDelegate {
            id: startupUi
            text: i18nc("@label:combobox", "Application behavior")
            description: i18nc("@info:description", "How the application should start and run.<br>Restart the program to apply.")
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            editable: false
            textRole: "label"
            valueRole: "key"
            model: [
                {
                    label: i18nc("@option", "Show system tray icon"),
                    key: "showTray"
                },
                {
                    label: i18nc("@option", "Show system tray icon & hide window on startup"),
                    key: "showTrayHideWindow"
                },
                {
                    label: i18nc("@option", "Hide system tray icon"),
                    key: "hideTray"
                }
            ]
            Component.onCompleted: currentIndex = indexOfValue(frontendManager.startupUi)
            onCurrentValueChanged: frontendManager.startupUi = currentValue
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Audio")
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: channels
            text: i18nc("@label:combobox", "Virtual channels")
            description: i18nc("@info:description", "Useful for games that only support 5.1 surround sound. Applied instantly.")
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            model: ["7.1", "5.1"]
            Component.onCompleted: currentIndex = indexOfValue(frontendManager.channels)
            onCurrentValueChanged: frontendManager.channels = currentValue
        }
    }
}
