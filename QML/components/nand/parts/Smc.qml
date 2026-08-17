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
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: root.enabled ? qsTr("Decrypted SMC Firmware Information") : qsTr("No NAND / SMC Loaded")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SMC Version:")
                text: typeof nandController !== "undefined" ? nandController.smcVersion : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SMC Type:")
                text: typeof nandController !== "undefined" ? nandController.smcType : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SMC Size:")
                text: typeof nandController !== "undefined" ? nandController.smcSize : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SMC Config Offset:")
                text: typeof nandController !== "undefined" ? nandController.smcConfigOffset : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
