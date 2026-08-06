import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Second Stage")

    property string cbAVersion: "9231"
    property string cbAPairing: "0x00000000"
    property string cbALockdown: "0x0000000F"
    property string cbBVersion: "9231"
    property string cbXVersion: "1111"

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: qsTr("Second Stage Bootloader (2BL)")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Section 1: CB_A / CB_X Group
        QQC2.Label {
            text: qsTr("CB_A / CB_X")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.05
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.mediumSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter

            QQC2.Label { text: qsTr("Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: root.cbAVersion; readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("Pairing Data:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: root.cbAPairing; readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("Lockdown Value:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: root.cbALockdown; readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }

            QQC2.Label { text: qsTr("CB_X Version (RGH3):"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: root.cbXVersion; readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Section 2: CB_B Group
        QQC2.Label {
            text: qsTr("CB_B")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.05
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.mediumSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter

            QQC2.Label { text: qsTr("Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
            QQC2.TextField { text: root.cbBVersion; readOnly: true; font.family: "Monospace"; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter; implicitWidth: Kirigami.Units.gridUnit * 14 }
        }

        Item { Layout.fillHeight: true }
    }
}
