import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import "nand"

Item {
    id: root

    property alias cpuKey: cpuKeyField.text
    property alias keyvaultPath: kvPathField.text
    property alias cbLdv: cbLdvSpinBox.value
    property alias cfLdv: cfLdvSpinBox.value
    property alias pairingData: pairingDataField.text
    property alias version: versionCombo.currentText
    property alias imageType: imageTypeCombo.currentText
    property alias consoleModel: consoleCombo.currentText
    property alias smcFile: smcCombo.currentText
    property var activePatches: []
    property var advancedOptions: ({})

    signal buildRequested(var config)

    function generateCpuKey() {
        var chars = "0123456789ABCDEF";
        var key = "";
        for (var i = 0; i < 32; i++) {
            key += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        cpuKeyField.text = key;
    }

    FileDialog {
        id: kvFileDialog
        title: qsTr("Select Keyvault File")
        nameFilters: [qsTr("Keyvault Files (*.bin *.kv)"), qsTr("All Files (*)")]
        onAccepted: {
            kvPathField.text = kvFileDialog.selectedFile.toString().replace("file://", "");
        }
    }

    NandMetadata {
        id: nand
    }

    PatchesDialog {
        id: patchesDialog
        availablePatches: nandBuilderController.availablePatches
        activePatches: root.activePatches
        onPatchesSaved: patches => root.activePatches = patches
    }

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

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

            QQC2.SpinBox {
                id: cbLdvSpinBox
                Kirigami.FormData.label: qsTr("CB LDV:")
                from: 0
                to: 80
                value: nand.cbLdv !== "" ? Number(nand.cbLdv) : 0
                editable: true
            }

            QQC2.SpinBox {
                id: cfLdvSpinBox
                Kirigami.FormData.label: qsTr("CF LDV:")
                from: 0
                to: 80
                value: 1
                editable: true
            }

            QQC2.TextField {
                id: pairingDataField
                Kirigami.FormData.label: qsTr("Pairing Data:")
                Layout.fillWidth: true
                text: nand.cbPairing
                placeholderText: qsTr("3 bytes, for example 0x123456")
                font.family: "Monospace"
                validator: RegularExpressionValidator {
                    regularExpression: /^(0[xX])?[0-9A-Fa-f]{6}$/
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }
            SelectorCombo {
                id: versionCombo
                Kirigami.FormData.label: qsTr("Version:")
                model: nandBuilderController.buildVersions
                onSelected: value => nandBuilderController.setSelectedVersion(value)
            }

            SelectorCombo {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                model: nandBuilderController.imageTypes
                onSelected: value => nandBuilderController.setSelectedImageType(value)
            }

            SelectorCombo {
                id: consoleCombo
                Kirigami.FormData.label: qsTr("Console:")
                model: nandBuilderController.consoles
                onSelected: value => nandBuilderController.setSelectedConsole(value)
            }

            SelectorCombo {
                id: smcCombo
                Kirigami.FormData.label: qsTr("SMC:")
                model: nandBuilderController.smcFiles
                onSelected: value => nandBuilderController.setSelectedSmc(value)
            }

            PatchesRow {
                Kirigami.FormData.label: qsTr("Patches & Options:")
                spacing: Kirigami.Units.mediumSpacing
                visible: versionCombo.currentText !== "" && imageTypeCombo.currentText !== "" && consoleCombo.currentText !== ""
                activePatches: root.activePatches
                patchesEnabled: !imageTypeCombo.currentText.startsWith("RGL-")
                showOptions: true
                onPatchesClicked: patchesDialog.open()
                onOptionsClicked: optionsDialog.open()
            }
        }

        Item {
            Layout.fillHeight: true
        }

        BuildButton {
            onClicked: {
                var config = {
                    "mode": "donor",
                    "donorMode": true,
                    "cpuKey": root.cpuKey,
                    "keyvaultPath": root.keyvaultPath,
                    "cbLdv": root.cbLdv,
                    "cfLdv": root.cfLdv,
                    "pairingData": root.pairingData,
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
