import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: nandSelector
    spacing: Kirigami.Units.largeSpacing
    Layout.fillWidth: true

    // NAND Selection Row
    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Controls.TextField {
            id: sourceField
            Layout.fillWidth: true
            readOnly: true
            placeholderText: qsTr("No NAND Selected...")
            palette.base: Kirigami.Theme.alternateBackgroundColor
        }

        Controls.Button {
            text: qsTr("Browse")
            icon.name: "document-open"
        }
    }

    // CPU Key Row
    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Controls.TextField {
            id: cpuKeyField
            Layout.fillWidth: true
            placeholderText: qsTr("CPU Key (32 Hex Characters)...")
            palette.base: Kirigami.Theme.alternateBackgroundColor
            maximumLength: 32
        }

        Controls.Button {
            text: qsTr("Import")
            icon.name: "key-enter"
        }
    }
}
