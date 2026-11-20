import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: root

    preferredWidth: Kirigami.Units.gridUnit * 24
    padding: Kirigami.Units.largeSpacing
    standardButtons: Kirigami.Dialog.Close

    // Array of { label: string, value: string, monospace?: bool, bold?: bool, placeholder?: string }
    property var fields: []

    Kirigami.FormLayout {
        id: detailsForm
        wideMode: true

        Repeater {
            model: root.fields

            QQC2.TextField {
                required property var modelData

                Kirigami.FormData.label: modelData.label || ""
                text: modelData.value || ""
                placeholderText: modelData.placeholder !== undefined ? modelData.placeholder : qsTr("Empty")
                readOnly: true
                font.bold: Boolean(modelData.bold)
                font.family: modelData.monospace ? "Monospace" : Kirigami.Theme.defaultFont.family
                Layout.fillWidth: true
            }
        }
    }
}
