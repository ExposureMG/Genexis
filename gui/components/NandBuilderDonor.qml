import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    property alias cpuKey: cpuKeyField.text
    property url keyvaultUrl
    property bool useGenericKeyvault: false
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
            root.useGenericKeyvault = false;
            root.keyvaultUrl = kvFileDialog.selectedFile;
        }
    }

    PatchesDialog {
        id: patchesDialog
        availablePatches: BuilderController.availablePatches
        selectedPatches: root.activePatches
        onPatchesAccepted: patches => root.activePatches = patches
    }

    Kirigami.Dialog {
        id: optionsDialog
        title: qsTr("Advanced xeBuild Options")
        preferredWidth: Kirigami.Units.gridUnit * 22
        padding: Kirigami.Units.largeSpacing
        standardButtons: Kirigami.Dialog.Save | Kirigami.Dialog.Cancel

        property var tempOptions: ({})

        onAboutToShow: {
            tempOptions = Object.assign({}, root.advancedOptions);
        }

        onAccepted: {
            root.advancedOptions = optionsDialog.tempOptions;
        }

        Kirigami.FormLayout {
            wideMode: true

            QQC2.CheckBox {
                Kirigami.FormData.label: qsTr("Disable FCrt:")
                checked: optionsDialog.tempOptions["nofcrt"] || false
                onToggled: optionsDialog.tempOptions["nofcrt"] = checked
            }

            QQC2.CheckBox {
                Kirigami.FormData.label: qsTr("Disable HDMI Wait:")
                checked: optionsDialog.tempOptions["nohdmiwait"] || false
                onToggled: optionsDialog.tempOptions["nohdmiwait"] = checked
            }

            QQC2.CheckBox {
                Kirigami.FormData.label: qsTr("Clean SMC:")
                checked: optionsDialog.tempOptions["cleanSmc"] || false
                onToggled: optionsDialog.tempOptions["cleanSmc"] = checked
            }
        }
    }

    Kirigami.FormLayout {

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
                    text: root.useGenericKeyvault ? qsTr("Generic Keyvault") : root.keyvaultUrl.toString()
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
                    onClicked: {
                        root.useGenericKeyvault = true;
                        root.keyvaultUrl = "";
                    }
                }
            }

            QQC2.SpinBox {
                id: cbLdvSpinBox
                Kirigami.FormData.label: qsTr("CB LDV:")
                from: 0
                to: 80
                value: BuilderController.defaultCbLdv
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
                text: BuilderController.defaultPairingData
                placeholderText: qsTr("3 bytes, for example 0x123456")
                font.family: "Monospace"
                validator: RegularExpressionValidator {
                    regularExpression: /^(0[xX])?[0-9A-Fa-f]{6}$/
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }
            QQC2.ComboBox {
                id: versionCombo
                Kirigami.FormData.label: qsTr("Version:")
                Layout.fillWidth: true
                model: BuilderController.buildVersions
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        BuilderController.setSelectedVersion(value);
                    }
                }
            }

            QQC2.ComboBox {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                Layout.fillWidth: true
                model: BuilderController.imageTypes
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        BuilderController.setSelectedImageType(value);
                    }
                }
            }

            QQC2.ComboBox {
                id: consoleCombo
                Kirigami.FormData.label: qsTr("Console:")
                Layout.fillWidth: true
                model: BuilderController.consoles
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        BuilderController.setSelectedConsole(value);
                    }
                }
            }

            QQC2.ComboBox {
                id: smcCombo
                Kirigami.FormData.label: qsTr("SMC:")
                Layout.fillWidth: true
                model: BuilderController.smcFiles
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        BuilderController.setSelectedSmc(value);
                    }
                }
            }

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

    footer: QQC2.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: buildButton
                text: qsTr("Build Image")
                icon.name: "system-run"
                highlighted: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 16

                onClicked: {
                    var config = {
                        "mode": "donor",
                        "buildType": "donor",
                        "xellOnly": false,
                        "donorMode": true,
                        "cpuKey": root.cpuKey,
                        "keyvaultUrl": root.keyvaultUrl,
                        "genericKeyvault": root.useGenericKeyvault,
                        "cbLdv": root.cbLdv,
                        "cfLdv": root.cfLdv,
                        "pairingData": root.pairingData,
                        "version": root.version,
                        "imageType": root.imageType,
                        "console": root.consoleModel,
                        "smc": root.smcFile,
                        "hack": "",
                        "image": "",
                        "patches": root.activePatches,
                        "options": root.advancedOptions
                    };
                    root.buildRequested(config);
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
