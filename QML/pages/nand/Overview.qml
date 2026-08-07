import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Image Overview")

    enabled: typeof nandController !== "undefined" && nandController.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: enabled ? qsTr("NAND Image Overview") : qsTr("No NAND File Loaded")
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
                text: qsTr("Image Size:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.imageSize : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("2BL / CB Version:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.cbVersion : ""
                placeholderText: qsTr("Empty")
                readOnly: true
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
                text: qsTr("Payload Indicator:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.payloadIndicator : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("Patch Slots:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.patchSlots : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        Item { Layout.fillHeight: true }
    }
}
