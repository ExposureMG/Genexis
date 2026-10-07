import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import ".."

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Second Stage")

    enabled: nandController.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

    NandMetadata {
        id: nand
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: root.enabled ? qsTr("Bootloader Chain (2BL / 3BL / 4BL / 5BL / Patch Slots)") : qsTr("No NAND File Loaded")
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
                Kirigami.FormData.label: qsTr("CB / CB_A Version:")
                text: nand.cbAVersion
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("CB_B Version:")
                text: nand.cbBVersion
                placeholderText: qsTr("N/A")
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("CB Size / Magic:")
                text: nand.cbSize !== "" ? qsTr("%1 bytes (%2)").arg(nand.cbSize).arg(nand.cbMagic) : ""
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("SC / CC (3BL) Version:")
                text: nand.scVersion !== "" ? nand.scVersion : nand.ccVersion
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("CD (4BL) Version:")
                text: nand.cdVersion
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("CE (5BL) Version:")
                text: nand.ceVersion
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("CF0 / CG0 Patch Version:")
                text: nand.cf0Version !== "" ? nand.cf0Version + " / " + nand.cg0Version : ""
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("CF1 / CG1 Patch Version:")
                text: nand.cf1Version !== "" ? nand.cf1Version + " / " + nand.cg1Version : ""
                font.family: "Monospace"
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
