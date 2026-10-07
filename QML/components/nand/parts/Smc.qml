import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import ".."

Kirigami.ScrollablePage {
    id: root

    title: qsTr("SMC Firmware")

    enabled: nandController.isNandLoaded && nandController.isSmcDecrypted
    opacity: enabled ? 1.0 : 0.45

    NandMetadata {
        id: nand
    }

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

            InfoField {
                Kirigami.FormData.label: qsTr("SMC Version:")
                text: nand.smcVersion
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("SMC Type:")
                text: nand.smcType
            }

            InfoField {
                Kirigami.FormData.label: qsTr("SMC Size:")
                text: nand.smcSize
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("SMC Config Offset:")
                text: nand.smcConfigOffset
                font.family: "Monospace"
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
