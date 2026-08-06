import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Settings")

    // Output properties for data binding / backend configuration
    property alias buildBackend: buildBackendCombo.currentText
    property alias flashBackend: flashBackendCombo.currentText
    property alias timingFlashBackend: timingBackendCombo.currentText
    property alias wirelessBackend: wirelessBackendCombo.currentText

    // Signals for Load & Save actions
    signal loadRequested()
    signal saveRequested(var config)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Dropdown 1: Build Backend
            QQC2.ComboBox {
                id: buildBackendCombo
                Kirigami.FormData.label: qsTr("Build Backend:")
                Layout.fillWidth: true
                model: ["gxbuild3", "Build360", "xeBuild", "RGBuild"]
            }

            // Dropdown 2: Flash Backend
            QQC2.ComboBox {
                id: flashBackendCombo
                Kirigami.FormData.label: qsTr("Flash Backend:")
                Layout.fillWidth: true
                model: ["NandProMax", "FTDI2SPI"]
            }

            // Dropdown 3: Timing Flash Backend
            QQC2.ComboBox {
                id: timingBackendCombo
                Kirigami.FormData.label: qsTr("Timing Flash Backend:")
                Layout.fillWidth: true
                model: ["NandProMax", "xsvftool"]
            }

            // Dropdown 4: Wireless Backend
            QQC2.ComboBox {
                id: wirelessBackendCombo
                Kirigami.FormData.label: qsTr("Wireless Backend:")
                Layout.fillWidth: true
                model: ["UpdClient"]
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // Pinned to bottom, same row: Load button & Save button
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 18
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.mediumSpacing

            QQC2.Button {
                id: loadButton
                text: qsTr("Load")
                icon.name: "document-open"
                Layout.fillWidth: true
                onClicked: root.loadRequested()
            }

            QQC2.Button {
                id: saveButton
                text: qsTr("Save")
                icon.name: "document-save"
                highlighted: true
                Layout.fillWidth: true
                onClicked: {
                    var config = {
                        "buildBackend": root.buildBackend,
                        "flashBackend": root.flashBackend,
                        "timingFlashBackend": root.timingFlashBackend,
                        "wirelessBackend": root.wirelessBackend
                    }
                    root.saveRequested(config)
                }
            }
        }
    }
}
