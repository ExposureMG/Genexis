import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Item {
    id: root

    // Output properties for parent/controller binding
    property alias cpuKey: cpuKeyField.text
    property alias keyvaultPath: kvPathField.text
    property alias cfLdv: cfLdvSpinBox.value
    property alias version: versionCombo.currentText
    property alias imageType: imageTypeCombo.currentText
    property alias consoleModel: consoleCombo.currentText
    property alias smcFile: smcCombo.currentText
    property var activePatches: []
    property var advancedOptions: ({})

    // Signal emitted when user clicks "Build Image"
    signal buildRequested(var config)

    // Helper to generate a random 32-character hex CPU Key
    function generateCpuKey() {
        var chars = "0123456789ABCDEF";
        var key = "";
        for (var i = 0; i < 32; i++) {
            key += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        cpuKeyField.text = key;
    }

    //  Keyvault File Dialog
    FileDialog {
        id: kvFileDialog
        title: qsTr("Select Keyvault File")
        nameFilters: [qsTr("Keyvault Files (*.bin *.kv)"), qsTr("All Files (*)")]
        onAccepted: {
            kvPathField.text = kvFileDialog.selectedFile.toString().replace("file://", "");
        }
    }

    //  Patches Dialog
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
            var states = {};
            var list = typeof nandBuilderController !== "undefined" ? nandBuilderController.availablePatches : [];
            for (var i = 0; i < list.length; i++) {
                var patchName = list[i];
                states[patchName] = root.activePatches.indexOf(patchName) !== -1;
            }
            tempStates = states;
        }

        contentItem: QQC2.ScrollView {
            clip: true
            ListView {
                id: patchesListView
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.availablePatches : []
                delegate: QQC2.CheckDelegate {
                    required property string modelData

                    width: patchesListView.width
                    text: modelData
                    checked: patchesDialog.tempStates[modelData] || false
                    onCheckedChanged: {
                        var updated = Object.assign({}, patchesDialog.tempStates);
                        updated[modelData] = checked;
                        patchesDialog.tempStates = updated;
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
                    var selected = [];
                    var list = typeof nandBuilderController !== "undefined" ? nandBuilderController.availablePatches : [];
                    for (var i = 0; i < list.length; i++) {
                        var patchName = list[i];
                        if (patchesDialog.tempStates[patchName]) {
                            selected.push(patchName);
                        }
                    }
                    root.activePatches = selected;
                    patchesDialog.accept();
                }
            }
        }
    }

    //  Advanced Options Dialog
    QQC2.Dialog {
        id: optionsDialog
        title: qsTr("Advanced xeBuild Options")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 22
        implicitHeight: Kirigami.Units.gridUnit * 20

        property var tempOptions: ({})

        onAboutToShow: {
            tempOptions = Object.assign({}, root.advancedOptions);
        }

        contentItem: QQC2.ScrollView {
            clip: true
            Kirigami.FormLayout {
                QQC2.CheckBox {
                    Kirigami.FormData.label: qsTr("Disable FCrt:")
                    checked: optionsDialog.tempOptions["nofcrt"] || false
                    onCheckedChanged: optionsDialog.tempOptions["nofcrt"] = checked
                }

                QQC2.CheckBox {
                    Kirigami.FormData.label: qsTr("Disable HDMI Wait:")
                    checked: optionsDialog.tempOptions["nohdmiwait"] || false
                    onCheckedChanged: optionsDialog.tempOptions["nohdmiwait"] = checked
                }

                QQC2.CheckBox {
                    Kirigami.FormData.label: qsTr("Clean SMC:")
                    checked: optionsDialog.tempOptions["cleanSmc"] || false
                    onCheckedChanged: optionsDialog.tempOptions["cleanSmc"] = checked
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
                    root.advancedOptions = optionsDialog.tempOptions;
                    optionsDialog.accept();
                }
            }
        }
    }

    //  Main Layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Row 1: CPU Key
            RowLayout {
                Kirigami.FormData.label: qsTr("CPU Key:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: cpuKeyField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Enter or generate 32-character CPU Key...")
                    font.family: "Monospace"
                    maximumLength: 32
                }

                QQC2.Button {
                    text: qsTr("Generate")
                    icon.name: "system-run"
                    onClicked: root.generateCpuKey()
                }
            }

            // Row 2: Keyvault
            RowLayout {
                Kirigami.FormData.label: qsTr("Keyvault:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: kvPathField
                    Layout.fillWidth: true
                    readOnly: true
                    placeholderText: qsTr("Select or generate Keyvault file...")
                }

                QQC2.Button {
                    text: qsTr("Browse")
                    icon.name: "document-open"
                    onClicked: kvFileDialog.open()
                }

                QQC2.Button {
                    text: qsTr("Generate")
                    icon.name: "document-new"
                    onClicked: kvPathField.text = qsTr("Generic Keyvault")
                }
            }

            // Row 3: CF LDV (Range 0 to 80)
            QQC2.SpinBox {
                id: cfLdvSpinBox
                Kirigami.FormData.label: qsTr("CF LDV:")
                from: 0
                to: 80
                value: 1
                editable: true
            }

            // Separator
            Kirigami.Separator {
                Layout.fillWidth: true
            }

            // NAND Configuration Dropdowns
            QQC2.ComboBox {
                id: versionCombo
                Kirigami.FormData.label: qsTr("Version:")
                Layout.fillWidth: true
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.buildVersions : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedVersion(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                Layout.fillWidth: true
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.imageTypes : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedImageType(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: consoleCombo
                Kirigami.FormData.label: qsTr("Console:")
                Layout.fillWidth: true
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.consoles : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedConsole(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: smcCombo
                Kirigami.FormData.label: qsTr("SMC:")
                Layout.fillWidth: true
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.smcFiles : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedSmc(currentText);
                    }
                }
            }

            // Row with Patches and Options Buttons
            RowLayout {
                Kirigami.FormData.label: qsTr("Patches & Options:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.mediumSpacing
                visible: versionCombo.currentText !== "" && imageTypeCombo.currentText !== "" && consoleCombo.currentText !== ""

                QQC2.Button {
                    id: patchesButton
                    enabled: !imageTypeCombo.currentText.startsWith("RGL-")
                    text: root.activePatches.length > 0 ? qsTr("Patches (%1 selected)").arg(root.activePatches.length) : qsTr("Patches")
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
                    "mode": "donor",
                    "cpuKey": root.cpuKey,
                    "keyvaultPath": root.keyvaultPath,
                    "cfLdv": root.cfLdv,
                    "version": root.version,
                    "imageType": root.imageType,
                    "consoleModel": root.consoleModel,
                    "smc": root.smcFile,
                    "patches": root.activePatches,
                    "options": root.advancedOptions
                };
                root.buildRequested(config);
            }
        }
    }
}
