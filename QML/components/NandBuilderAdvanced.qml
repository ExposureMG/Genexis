import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    // Output properties for parent/controller binding
    property alias buildType: buildTypeCombo.currentText
    
    // NAND Image / Donor specific properties
    property alias version: versionCombo.currentText
    property alias imageType: imageTypeCombo.currentText
    property alias consoleModel: consoleCombo.currentText

    // XeLL Image specific properties
    property alias xellHack: xellHackCombo.currentText
    property alias xellImage: xellImageCombo.currentText

    // Selected patches list
    property var activePatches: []

    // Advanced options property object
    property var advancedOptions: ({
        "cpuKey": "",
        "kvPath": "",
        "smcPath": "",
        "customArgs": ""
    })

    // Signal emitted when user clicks "Build Image"
    signal buildRequested(var config)

    // Helper property to check current build type mode
    readonly property bool isXeLL: buildTypeCombo.currentText === qsTr("XeLL Image")

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

    // ── Advanced Options Dialog ─────────────────────────────────────────
    QQC2.Dialog {
        id: optionsDialog
        title: qsTr("Advanced Image Options")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 18

        property string tempCpuKey: ""
        property string tempKvPath: ""
        property string tempSmcPath: ""
        property string tempCustomArgs: ""

        onAboutToShow: {
            tempCpuKey = root.advancedOptions.cpuKey || ""
            tempKvPath = root.advancedOptions.kvPath || ""
            tempSmcPath = root.advancedOptions.smcPath || ""
            tempCustomArgs = root.advancedOptions.customArgs || ""
        }

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: optionsDialog.width - optionsDialog.padding * 2

                // CPU Key
                QQC2.Label {
                    text: qsTr("CPU Key:")
                    font.bold: true
                    color: Kirigami.Theme.disabledTextColor
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
                QQC2.TextField {
                    id: cpuKeyField
                    text: optionsDialog.tempCpuKey
                    placeholderText: qsTr("32-digit Hex CPU Key")
                    font.family: "Monospace"
                    Layout.fillWidth: true
                    onTextChanged: optionsDialog.tempCpuKey = text
                }

                // Keyvault File Path
                QQC2.Label {
                    text: qsTr("Keyvault (KV):")
                    font.bold: true
                    color: Kirigami.Theme.disabledTextColor
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
                QQC2.TextField {
                    id: kvPathField
                    text: optionsDialog.tempKvPath
                    placeholderText: qsTr("Path to kv.bin…")
                    Layout.fillWidth: true
                    onTextChanged: optionsDialog.tempKvPath = text
                }

                // SMC File Path
                QQC2.Label {
                    text: qsTr("SMC File:")
                    font.bold: true
                    color: Kirigami.Theme.disabledTextColor
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
                QQC2.TextField {
                    id: smcPathField
                    text: optionsDialog.tempSmcPath
                    placeholderText: qsTr("Path to smc.bin…")
                    Layout.fillWidth: true
                    onTextChanged: optionsDialog.tempSmcPath = text
                }

                // Custom Arguments / Parameters
                QQC2.Label {
                    text: qsTr("Custom Flags:")
                    font.bold: true
                    color: Kirigami.Theme.disabledTextColor
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
                QQC2.TextField {
                    id: customArgsField
                    text: optionsDialog.tempCustomArgs
                    placeholderText: qsTr("Extra build flags…")
                    Layout.fillWidth: true
                    onTextChanged: optionsDialog.tempCustomArgs = text
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
                    root.advancedOptions = {
                        "cpuKey": optionsDialog.tempCpuKey,
                        "kvPath": optionsDialog.tempKvPath,
                        "smcPath": optionsDialog.tempSmcPath,
                        "customArgs": optionsDialog.tempCustomArgs
                    }
                    optionsDialog.accept()
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
                model: [qsTr("NAND Image"), qsTr("XeLL Image"), qsTr("Donor")]
            }

            // XeLL Image Mode Controls
            QQC2.ComboBox {
                id: xellHackCombo
                Kirigami.FormData.label: qsTr("Hack:")
                Layout.fillWidth: true
                visible: root.isXeLL
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.xellHacks : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedXellHack(currentText)
                    }
                }
            }

            QQC2.ComboBox {
                id: xellImageCombo
                Kirigami.FormData.label: qsTr("Image:")
                Layout.fillWidth: true
                visible: root.isXeLL
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.xellImages : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedXellImage(currentText)
                    }
                }
            }

            // NAND Image / Donor Mode Controls
            QQC2.ComboBox {
                id: versionCombo
                Kirigami.FormData.label: qsTr("Version:")
                Layout.fillWidth: true
                visible: !root.isXeLL
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.buildVersions : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedVersion(currentText)
                    }
                }
            }

            QQC2.ComboBox {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                Layout.fillWidth: true
                visible: !root.isXeLL
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.imageTypes : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedImageType(currentText)
                    }
                }
            }

            QQC2.ComboBox {
                id: consoleCombo
                Kirigami.FormData.label: qsTr("Console:")
                Layout.fillWidth: true
                visible: !root.isXeLL
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.consoles : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedConsole(currentText)
                    }
                }
            }

            // Row with Patches and Options Buttons on the same row
            RowLayout {
                Kirigami.FormData.label: qsTr("Patches & Options:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.mediumSpacing

                QQC2.Button {
                    id: patchesButton
                    text: root.activePatches.length > 0
                          ? qsTr("Patches (%1 selected)").arg(root.activePatches.length)
                          : qsTr("Patches")
                    icon.name: "preferences-system-patches"
                    onClicked: patchesDialog.open()
                }

                QQC2.Button {
                    id: optionsButton
                    text: qsTr("Options")
                    icon.name: "configure"
                    onClicked: optionsDialog.open()
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
                    "patches": root.activePatches,
                    "options": root.advancedOptions
                }
                if (root.isXeLL) {
                    config["hack"] = root.xellHack
                    config["image"] = root.xellImage
                } else {
                    config["version"] = root.version
                    config["imageType"] = root.imageType
                    config["console"] = root.consoleModel
                }
                root.buildRequested(config)
            }
        }
    }
}
