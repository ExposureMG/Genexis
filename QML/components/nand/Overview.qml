import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Image Overview")

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
            text: root.enabled ? qsTr("Image Overview") : qsTr("No NAND Image Loaded")
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
                Kirigami.FormData.label: qsTr("Image Size:")
                text: nand.imageSize
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("Block Type:")
                text: nand.blockType
                font.family: "Monospace"
            }

            InfoField {
                Kirigami.FormData.label: qsTr("SMC Type:")
                text: nand.smcType
            }

            InfoField {
                Kirigami.FormData.label: qsTr("Kernel:")
                text: nand.kernelVerOrType
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
