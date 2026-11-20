import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Image Overview")

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        visible: !NandSession.isNandLoaded
        icon.name: "drive-harddisk"
        text: qsTr("No NAND Image Loaded")
        explanation: qsTr("Open a NAND image file to inspect its overview.")
    }

    Kirigami.FormLayout {
        visible: NandSession.isNandLoaded

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
}
