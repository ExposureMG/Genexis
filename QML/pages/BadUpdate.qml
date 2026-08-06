import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Bad Update")

    // Output properties for component binding
    property alias entry: entryCombo.currentText
    property alias payload: payloadCombo.currentText
    property var activePatches: []

    // Signal emitted when user confirms Save As
    signal saveRequested(string filePath, var config)

    // ── Patches Dialog ──────────────────────────────────────────────────
    QQC2.Dialog {
        id: patchesDialog
        title: qsTr("Configure Patches")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 18
        implicitHeight: Kirigami.Units.gridUnit * 16

        property var tempStates: ({})

        onAboutToShow: {
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

    // ── File Save Dialog ────────────────────────────────────────────────
    FileDialog {
        id: saveFileDialog
        title: qsTr("Save As")
        fileMode: FileDialog.SaveFile
        onAccepted: {
            var config = {
                "entry": root.entry,
                "payload": root.payload,
                "patches": root.activePatches
            }
            console.log("BadUpdate Save As requested for file:", selectedFile, JSON.stringify(config))
            root.saveRequested(selectedFile, config)
        }
    }

    // ── Main Page Layout ────────────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Dropdown 1: Entry
            QQC2.ComboBox {
                id: entryCombo
                Kirigami.FormData.label: qsTr("Entry:")
                Layout.fillWidth: true
                model: ["Rock Band Blitz", "Avatar"]
            }

            // Dropdown 2: Payload
            QQC2.ComboBox {
                id: payloadCombo
                Kirigami.FormData.label: qsTr("Payload:")
                Layout.fillWidth: true
                model: ["FreeMyXe", "XeUnshackle"]
            }

            // Patches Button Row
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

        // Save As Button at bottom matching NandBuilder / Flasher style
        QQC2.Button {
            id: saveAsButton
            text: qsTr("Save As…")
            icon.name: "system-run"
            highlighted: true
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter
            onClicked: saveFileDialog.open()
        }
    }
}
