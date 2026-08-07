import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("SMC Firmware")

    enabled: typeof nandController !== "undefined" && nandController.isNandLoaded && nandController.isSmcDecrypted
    opacity: enabled ? 1.0 : 0.45

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: enabled ? qsTr("Decrypted SMC Firmware Information") : qsTr("No NAND / SMC Loaded")
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

            QQC2.Label {
                text: qsTr("SMC Version:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.smcVersion : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("SMC Type:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.smcType : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("SMC Size:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.smcSize : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("SMC Config Offset:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.smcConfigOffset : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        Item { Layout.fillHeight: true }
    }
}
