import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: qsTr("Settings")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Card {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 40
            Layout.alignment: Qt.AlignHCenter
            
            header: Kirigami.Heading {
                text: qsTr("General Configuration")
                level: 3
            }
            
            contentItem: Kirigami.FormLayout {
                Controls.TextField {
                    Kirigami.FormData.label: qsTr("Workspace Path:")
                    placeholderText: "~/Genexis"
                    Layout.fillWidth: true
                }

                Controls.ComboBox {
                    Kirigami.FormData.label: qsTr("Interface Style:")
                    model: ["Kirigami Default", "Breeze", "Fusion", "Material"]
                    Layout.fillWidth: true
                }

                Controls.CheckBox {
                    Kirigami.FormData.label: qsTr("Auto-save Log:")
                    checked: true
                }

                Kirigami.Separator {
                    Kirigami.FormData.isSection: true
                    Kirigami.FormData.label: qsTr("Build System")
                }

                Controls.TextField {
                    Kirigami.FormData.label: qsTr("xeBuild Path:")
                    placeholderText: "/usr/bin/xebuild"
                    Layout.fillWidth: true
                }

                Controls.CheckBox {
                    Kirigami.FormData.label: qsTr("Verbose Output:")
                }
            }
        }

        Kirigami.Card {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 40
            Layout.alignment: Qt.AlignHCenter
            
            header: Kirigami.Heading {
                text: qsTr("Hardware Interface")
                level: 3
            }

            contentItem: Kirigami.FormLayout {
                Controls.ComboBox {
                    Kirigami.FormData.label: qsTr("Default Flasher:")
                    model: ["Auto-detect", "FTDI2SPI", "NAND-X", "JR-Programmer"]
                    Layout.fillWidth: true
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
