import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Second Stage")

    enabled: typeof nandController !== "undefined" && nandController.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: enabled ? qsTr("Bootloader Chain (2BL / 3BL / 4BL / 5BL / Patch Slots)") : qsTr("No NAND File Loaded")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.mediumSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter

            QQC2.Label { text: qsTr("CB / CB_A Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" ? nandController.cbAVersion : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CB_B Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" ? nandController.cbBVersion : ""; placeholderText: qsTr("N/A"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CB Size / Magic:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" && nandController.cbSize !== "" ? nandController.cbSize + " bytes (" + nandController.cbMagic + ")" : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("SC / CC (3BL) Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" ? (nandController.scVersion !== "" ? nandController.scVersion : nandController.ccVersion) : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CD (4BL) Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" ? nandController.cdVersion : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CE (5BL) Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" ? nandController.ceVersion : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CF0 / CG0 Patch Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" && nandController.cf0Version !== "" ? nandController.cf0Version + " / " + nandController.cg0Version : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CF1 / CG1 Patch Version:"); font.bold: true; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: typeof nandController !== "undefined" && nandController.cf1Version !== "" ? nandController.cf1Version + " / " + nandController.cg1Version : ""; placeholderText: qsTr("Empty"); readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }
        }

        Item { Layout.fillHeight: true }
    }
}
