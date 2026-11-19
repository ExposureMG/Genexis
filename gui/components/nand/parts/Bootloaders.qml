import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Second Stage")

    enabled: NandSession.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

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

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CB / CB_A Version:")
                text: InfoController.cbAVersion
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CB_B Version:")
                text: InfoController.cbBVersion
                placeholderText: qsTr("N/A")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CB Size / Magic:")
                text: InfoController.cbSize !== "" ? InfoController.cbSize + " bytes (" + InfoController.cbMagic + ")" : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SC / CC (3BL) Version:")
                text: InfoController.scVersion !== "" ? InfoController.scVersion : InfoController.ccVersion
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CD (4BL) Version:")
                text: InfoController.cdVersion
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CE (5BL) Version:")
                text: InfoController.ceVersion
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CF0 / CG0 Patch Version:")
                text: InfoController.cf0Version !== "" ? InfoController.cf0Version + " / " + InfoController.cg0Version : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CF1 / CG1 Patch Version:")
                text: InfoController.cf1Version !== "" ? InfoController.cf1Version + " / " + InfoController.cg1Version : ""
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
