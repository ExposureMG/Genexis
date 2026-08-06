import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    // Output properties for parent/controller binding
    property alias buildType: buildTypeCombo.currentText
    property alias buildVersion: buildVersionCombo.currentText
    property alias hack: hackCombo.currentText
    property alias hackVersion: hackVersionCombo.currentText
    property var activePatches: []

    // Signal emitted when user clicks "Build Image"
    signal buildRequested(var config)

    // Helper model for hack versions based on selected hack
    function getHackVersionModel(selectedHack) {
        if (selectedHack === qsTr("RGH")) {
            return ["RGH 1", "RGH 2", "RGH 1.2", "S-RGH", "RGH 3", "RGH 1.3"]
        } else if (selectedHack === qsTr("JTAG")) {
            return ["Argon Data", "AUD_CLAMP", "R-JTAG"]
        }
        return ["None"]
    }

    // ── Patches Dialog ──────────────────────────────────────────────────
    QQC2.Dialog {
        id: patchesDialog
        title: qsTr("Configure Patches")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 18
        implicitHeight: Kirigami.Units.gridUnit * 16

        // Temporary check states while dialog is open
        property var tempStates: ({})

        onAboutToShow: {
            // Copy activePatches state to tempStates
            var states = {}
            for (var i = 0; i < patchesModel.count; i++) {
                var patchName = patchesModel.get(i).name
                states[patchName] = root.activePatches.indexOf(patchName) !== -1
            }
            tempStates = states
        }

        ListModel {
            id: patchesModel
            ListElement { name: "nohdmiwait"; label: "nohdmiwait" }
            ListElement { name: "USBdsec"; label: "USBdsec" }
        }

        contentItem: QQC2.ScrollView {
            clip: true
            ListView {
                id: patchesListView
                model: patchesModel
                delegate: QQC2.CheckDelegate {
                    required property string name
                    required property string label
                    required property int index

                    width: patchesListView.width
                    text: label
                    checked: patchesDialog.tempStates[name] || false
                    onCheckedChanged: {
                        var updated = Object.assign({}, patchesDialog.tempStates)
                        updated[name] = checked
                        patchesDialog.tempStates = updated
                    }
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight

            QQC2.Button {
                text: qsTr("Close")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: patchesDialog.reject()
            }

            QQC2.Button {
                text: qsTr("Save")
                highlighted: true
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    var selected = []
                    for (var i = 0; i < patchesModel.count; i++) {
                        var patchName = patchesModel.get(i).name
                        if (patchesDialog.tempStates[patchName]) {
                            selected.push(patchName)
                        }
                    }
                    root.activePatches = selected
                    patchesDialog.accept()
                }
            }
        }
    }

    // ── Main Layout ─────────────────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Dropdown 1: Build Type
            QQC2.ComboBox {
                id: buildTypeCombo
                Kirigami.FormData.label: qsTr("Build Type:")
                Layout.fillWidth: true
                model: [qsTr("NAND Image"), qsTr("XeLL Image")]
            }

            // Dropdown 2: Build Version (Disabled if XeLL is selected)
            QQC2.ComboBox {
                id: buildVersionCombo
                Kirigami.FormData.label: qsTr("Build Version:")
                Layout.fillWidth: true
                enabled: buildTypeCombo.currentText !== qsTr("XeLL Image")
                model: [qsTr("Latest")]
            }

            // Dropdown 3: Hack
            QQC2.ComboBox {
                id: hackCombo
                Kirigami.FormData.label: qsTr("Hack:")
                Layout.fillWidth: true
                model: [qsTr("None"), qsTr("RGH"), qsTr("JTAG")]
                onCurrentTextChanged: {
                    hackVersionCombo.model = root.getHackVersionModel(currentText)
                    hackVersionCombo.currentIndex = 0
                }
            }

            // Dropdown 4: Hack Version (Dynamic based on Hack)
            QQC2.ComboBox {
                id: hackVersionCombo
                Kirigami.FormData.label: qsTr("Hack Version:")
                Layout.fillWidth: true
                enabled: hackCombo.currentText !== qsTr("None")
                model: root.getHackVersionModel(hackCombo.currentText)
            }

            // Patches Button
            RowLayout {
                Kirigami.FormData.label: qsTr("Patches:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    id: patchesButton
                    text: root.activePatches.length > 0
                          ? qsTr("Patches (%1 selected)").arg(root.activePatches.length)
                          : qsTr("Patches")
                    icon.name: "preferences-system-patches"
                    onClicked: patchesDialog.open()
                }

                QQC2.Label {
                    visible: root.activePatches.length > 0
                    text: root.activePatches.join(", ")
                    color: Kirigami.Theme.disabledTextColor
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // Build Image Button
        QQC2.Button {
            id: buildButton
            text: qsTr("Build Image")
            icon.name: "system-run"
            highlighted: true
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                var config = {
                    "buildType": root.buildType,
                    "buildVersion": root.buildType === qsTr("XeLL Image") ? "" : root.buildVersion,
                    "hack": root.hack,
                    "hackVersion": root.hack === qsTr("None") ? "" : root.hackVersion,
                    "patches": root.activePatches
                }
                root.buildRequested(config)
            }
        }
    }
}
