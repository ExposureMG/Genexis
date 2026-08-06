import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Flasher")

    property string selectedFilePath: ""
    property alias operation: operationCombo.currentText
    property string logText: qsTr("[INFO] Flasher initialized.\n[INFO] Ready for operation.\n")

    // Timings options state
    property var timingOptions: ({
        "preset": "Default",
        "delayMs": 0
    })

    // Flasher options state
    property var flasherOptions: ({
        "verify": true,
        "dumpCount": 2,
        "device": "Auto"
    })

    // ── Timings Dialog ──────────────────────────────────────────────────
    QQC2.Dialog {
        id: timingsDialog
        title: qsTr("Timings")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 20
        implicitHeight: Kirigami.Units.gridUnit * 14

        property string tempPreset: root.timingOptions.preset || "Default"
        property int tempDelay: root.timingOptions.delayMs || 0

        contentItem: QQC2.ScrollView {
            clip: true
            ColumnLayout {
                spacing: Kirigami.Units.mediumSpacing
                width: timingsDialog.width - timingsDialog.padding * 2

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    QQC2.ComboBox {
                        id: timingPresetCombo
                        Kirigami.FormData.label: qsTr("Timing Preset:")
                        Layout.fillWidth: true
                        model: ["Default", "Fast", "Safe", "Custom"]
                        currentIndex: model.indexOf(timingsDialog.tempPreset) !== -1 ? model.indexOf(timingsDialog.tempPreset) : 0
                        onCurrentTextChanged: timingsDialog.tempPreset = currentText
                    }

                    QQC2.SpinBox {
                        id: timingDelaySpin
                        Kirigami.FormData.label: qsTr("Delay (ms):")
                        from: 0
                        to: 1000
                        stepSize: 10
                        value: timingsDialog.tempDelay
                        onValueChanged: timingsDialog.tempDelay = value
                    }
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight

            QQC2.Button {
                text: qsTr("Close")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: timingsDialog.reject()
            }

            QQC2.Button {
                text: qsTr("Save")
                highlighted: true
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    root.timingOptions = {
                        "preset": timingsDialog.tempPreset,
                        "delayMs": timingsDialog.tempDelay
                    }
                    timingsDialog.accept()
                }
            }
        }
    }

    // ── Options Dialog ──────────────────────────────────────────────────
    QQC2.Dialog {
        id: optionsDialog
        title: qsTr("Flasher Options")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 22
        implicitHeight: Kirigami.Units.gridUnit * 16

        property bool tempVerify: root.flasherOptions.verify
        property int tempDumpCount: root.flasherOptions.dumpCount
        property string tempDevice: root.flasherOptions.device

        contentItem: QQC2.ScrollView {
            clip: true
            ColumnLayout {
                spacing: Kirigami.Units.mediumSpacing
                width: optionsDialog.width - optionsDialog.padding * 2

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    QQC2.ComboBox {
                        id: deviceCombo
                        Kirigami.FormData.label: qsTr("Hardware Device:")
                        Layout.fillWidth: true
                        model: ["Auto", "PicoFlasher", "J-Runner Programmer", "NAND-X", "SQUIRT"]
                        onCurrentTextChanged: optionsDialog.tempDevice = currentText
                    }

                    QQC2.SpinBox {
                        id: dumpCountSpin
                        Kirigami.FormData.label: qsTr("Read Dumps Count:")
                        from: 1
                        to: 4
                        value: optionsDialog.tempDumpCount
                        onValueChanged: optionsDialog.tempDumpCount = value
                    }

                    QQC2.CheckBox {
                        id: verifyCheck
                        Kirigami.FormData.label: qsTr("Verification:")
                        text: qsTr("Verify after write")
                        checked: optionsDialog.tempVerify
                        onCheckedChanged: optionsDialog.tempVerify = checked
                    }
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight

            QQC2.Button {
                text: qsTr("Close")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: optionsDialog.reject()
            }

            QQC2.Button {
                text: qsTr("Save")
                highlighted: true
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    root.flasherOptions = {
                        "verify": optionsDialog.tempVerify,
                        "dumpCount": optionsDialog.tempDumpCount,
                        "device": optionsDialog.tempDevice
                    }
                    optionsDialog.accept()
                }
            }
        }
    }

    // ── File Selection Dialog ───────────────────────────────────────────
    FileDialog {
        id: filePicker
        title: qsTr("Select NAND File")
        onAccepted: root.selectedFilePath = selectedFile
    }

    // ── Main Page Layout ────────────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Black output console
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Kirigami.Units.gridUnit * 12
            color: "#1e1e1e"
            radius: Kirigami.Units.smallSpacing
            border.color: Qt.rgba(1, 1, 1, 0.1)

            QQC2.ScrollView {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                clip: true

                QQC2.TextArea {
                    id: consoleText
                    text: root.logText
                    readOnly: true
                    font.family: "Monospace"
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: "#00ff66"
                    background: null
                    selectByMouse: true
                    wrapMode: Text.Wrap
                }
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Selected File row
            RowLayout {
                Kirigami.FormData.label: qsTr("Selected File:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: selectedFileField
                    text: root.selectedFilePath
                    placeholderText: qsTr("No file selected…")
                    readOnly: true
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    text: qsTr("Browse…")
                    onClicked: filePicker.open()
                }
            }

            // Timings & Options Buttons on the same row
            RowLayout {
                Kirigami.FormData.label: qsTr("Configuration:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.mediumSpacing

                QQC2.Button {
                    id: timingsButton
                    text: qsTr("Timings")
                    icon.name: "preferences-system-time"
                    onClicked: timingsDialog.open()
                }

                QQC2.Button {
                    id: optionsButton
                    text: qsTr("Options")
                    icon.name: "configure"
                    onClicked: optionsDialog.open()
                }

                Item { Layout.fillWidth: true }
            }

            // Operation Dropdown
            QQC2.ComboBox {
                id: operationCombo
                Kirigami.FormData.label: qsTr("Operation:")
                Layout.fillWidth: true
                model: [qsTr("Read"), qsTr("Write")]
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // Action Button at bottom matching NandBuilder style
        QQC2.Button {
            id: actionButton
            text: operationCombo.currentText === qsTr("Read")
                  ? qsTr("Read NAND")
                  : qsTr("Write NAND")
            icon.name: "system-run"
            highlighted: true
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                var timestamp = new Date().toLocaleTimeString()
                root.logText += "[" + timestamp + "] Initiating " + operationCombo.currentText + " operation...\n"
                if (root.selectedFilePath !== "") {
                    root.logText += "[" + timestamp + "] Target file: " + root.selectedFilePath + "\n"
                }
            }
        }
    }
}
