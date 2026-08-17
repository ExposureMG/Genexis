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
                text: typeof nandController !== "undefined" ? nandController.cbAVersion : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CB_B Version:")
                text: typeof nandController !== "undefined" ? nandController.cbBVersion : ""
                placeholderText: qsTr("N/A")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CB Size / Magic:")
                text: typeof nandController !== "undefined" && nandController.cbSize !== "" ? nandController.cbSize + " bytes (" + nandController.cbMagic + ")" : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("SC / CC (3BL) Version:")
                text: typeof nandController !== "undefined" ? (nandController.scVersion !== "" ? nandController.scVersion : nandController.ccVersion) : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CD (4BL) Version:")
                text: typeof nandController !== "undefined" ? nandController.cdVersion : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CE (5BL) Version:")
                text: typeof nandController !== "undefined" ? nandController.ceVersion : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CF0 / CG0 Patch Version:")
                text: typeof nandController !== "undefined" && nandController.cf0Version !== "" ? nandController.cf0Version + " / " + nandController.cg0Version : ""
                placeholderText: qsTr("Empty")
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("CF1 / CG1 Patch Version:")
                text: typeof nandController !== "undefined" && nandController.cf1Version !== "" ? nandController.cf1Version + " / " + nandController.cg1Version : ""
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
