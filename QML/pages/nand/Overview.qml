import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Image Overview")

    property string flashType: "Trinity 16MB"
    property string blockType: "Small Block"
    property string imageType: "Glitch3 / RGH3"

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: qsTr("NAND Image Overview")
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
                text: qsTr("Flash Type:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.flashType
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("Block Type:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.blockType
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("Image Type:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.imageType
                readOnly: true
                font.bold: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        Item { Layout.fillHeight: true }
    }
}
