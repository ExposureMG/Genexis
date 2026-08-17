import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("System Updates")

    property string cf0Version: "17559"
    property string cf0Pairing: "000000"
    property string cf0Lockdown: "12"
    property string cg0Version: "17559"
    property string cf1Version: "17559"
    property string cf1Pairing: "000000"
    property string cf1Lockdown: "12"
    property string cg1Version: "17559"

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: qsTr("System Update")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing * 2

            
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: Kirigami.Units.mediumSpacing

                QQC2.Label {
                    text: qsTr("Patchslot 0")
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.05
                    Layout.alignment: Qt.AlignHCenter
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }

                QQC2.Label {
                    text: qsTr("CF")
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
                        text: qsTr("Version:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cf0Version
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }

                    QQC2.Label {
                        text: qsTr("Pairing:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cf0Pairing
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }

                    QQC2.Label {
                        text: qsTr("Lockdown:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cf0Lockdown
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }
                }

                QQC2.Label {
                    text: qsTr("CG")
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
                        text: qsTr("Version:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cg0Version
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }
                }
            }

            Kirigami.Separator {
                Layout.fillHeight: true
            }

            
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: Kirigami.Units.mediumSpacing

                QQC2.Label {
                    text: qsTr("Patchslot 1 (CF1 / CG1)")
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.05
                    Layout.alignment: Qt.AlignHCenter
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }

                QQC2.Label {
                    text: qsTr("CF")
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
                        text: qsTr("Version:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cf1Version
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }

                    QQC2.Label {
                        text: qsTr("Pairing:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cf1Pairing
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }

                    QQC2.Label {
                        text: qsTr("Lockdown:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cf1Lockdown
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }
                }

                QQC2.Label {
                    text: qsTr("CG")
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
                        text: qsTr("Version:")
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    QQC2.TextField {
                        text: root.cg1Version
                        readOnly: true
                        font.family: "Monospace"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        implicitWidth: Kirigami.Units.gridUnit * 12
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
