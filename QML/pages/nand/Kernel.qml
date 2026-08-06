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
        width: parent.width

        QQC2.Label {
            text: qsTr("Stage 3 and Kernel Information")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: qsTr("Stage 3")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.0
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.mediumSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter

            QQC2.Label {
                text: qsTr("Type:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.stage3Type
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("Version:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.stage3Version
                readOnly: true
                font.family: "Monospace"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        QQC2.Label {
            text: qsTr("Kernel")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.0
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.mediumSpacing
            Layout.alignment: Qt.AlignHCenter

            QQC2.Label {
                text: qsTr("Type:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.kernelType
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            QQC2.Label {
                text: qsTr("Kernel Version:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.kernelVersion
                readOnly: true
                font.bold: true
                font.family: "Monospace"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        Item { Layout.fillHeight: true }
    }
}
