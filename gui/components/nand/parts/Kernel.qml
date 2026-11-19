import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Stage 3 & Kernel")

    property string stage3Type: "CD"
    property string stage3Version: "8453"
    property string kernelType: "CE"
    property string kernelVersion: "17559"

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: qsTr("Stage 3 and Kernel Information")
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
                Kirigami.FormData.label: qsTr("Stage 3 Type:")
                text: root.stage3Type
                readOnly: true
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Stage 3 Version:")
                text: root.stage3Version
                readOnly: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Kernel Type:")
                text: root.kernelType
                readOnly: true
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Kernel Version:")
                text: root.kernelVersion
                readOnly: true
                font.bold: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
