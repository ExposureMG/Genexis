import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Image Overview")

    enabled: NandSession.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

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

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Image Size:")
                text: NandSession.imageSize
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Block Type:")
                text: InfoController.blockType
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SMC Type:")
                text: InfoController.smcType
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Kernel:")
                text: InfoController.kernelVerOrType
                placeholderText: qsTr("Empty")
                readOnly: true
                Layout.fillWidth: true
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
