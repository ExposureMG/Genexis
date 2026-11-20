import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    property alias buildType: buildTypeCombo.currentValue

    property alias version: versionCombo.currentText
    property alias imageType: imageTypeCombo.currentText
    property alias consoleModel: consoleCombo.currentText
    property alias smcFile: smcCombo.currentText
    property alias xellHack: xellHackCombo.currentText
    property alias xellImage: xellImageCombo.currentText

    property var activePatches: []

    property var advancedOptions: ({
            "cpuKey": "",
            "kvPath": "",
            "smcPath": "",
            "customArgs": ""
        })

    signal buildRequested(var config)

    readonly property bool isXeLL: buildTypeCombo.currentValue === "xell"
    readonly property bool isDonor: buildTypeCombo.currentValue === "donor"

    PatchesDialog {
        id: patchesDialog
        availablePatches: BuilderController.availablePatches
        selectedPatches: root.activePatches
        onPatchesAccepted: patches => root.activePatches = patches
    }

    Kirigami.Dialog {
        id: optionsDialog
        title: qsTr("Advanced Image Options")
        preferredWidth: Kirigami.Units.gridUnit * 24
        padding: Kirigami.Units.largeSpacing
        standardButtons: Kirigami.Dialog.Save | Kirigami.Dialog.Cancel

        property string tempCpuKey: ""
        property string tempKvPath: ""
        property string tempSmcPath: ""
        property string tempCustomArgs: ""

        onAboutToShow: {
            tempCpuKey = root.advancedOptions.cpuKey || "";
            tempKvPath = root.advancedOptions.kvPath || "";
            tempSmcPath = root.advancedOptions.smcPath || "";
            tempCustomArgs = root.advancedOptions.customArgs || "";
        }

        onAccepted: {
            root.advancedOptions = {
                "cpuKey": optionsDialog.tempCpuKey,
                "kvPath": optionsDialog.tempKvPath,
                "smcPath": optionsDialog.tempSmcPath,
                "customArgs": optionsDialog.tempCustomArgs
            };
        }

        Kirigami.FormLayout {
            wideMode: true

            QQC2.TextField {
                id: cpuKeyField
                Kirigami.FormData.label: qsTr("CPU Key:")
                text: optionsDialog.tempCpuKey
                placeholderText: qsTr("32-digit Hex CPU Key")
                font.family: "Monospace"
                Layout.fillWidth: true
                onTextEdited: optionsDialog.tempCpuKey = text
            }

            QQC2.TextField {
                id: kvPathField
                Kirigami.FormData.label: qsTr("Keyvault (KV):")
                text: optionsDialog.tempKvPath
                placeholderText: qsTr("Path to kv.bin…")
                Layout.fillWidth: true
                onTextEdited: optionsDialog.tempKvPath = text
            }

            QQC2.TextField {
                id: smcPathField
                Kirigami.FormData.label: qsTr("SMC File:")
                text: optionsDialog.tempSmcPath
                placeholderText: qsTr("Path to smc.bin…")
                Layout.fillWidth: true
                onTextEdited: optionsDialog.tempSmcPath = text
            }

            QQC2.TextField {
                id: customArgsField
                Kirigami.FormData.label: qsTr("Custom Flags:")
                text: optionsDialog.tempCustomArgs
                placeholderText: qsTr("Extra build flags…")
                Layout.fillWidth: true
                onTextEdited: optionsDialog.tempCustomArgs = text
            }
        }
    }

    Kirigami.FormLayout {

            QQC2.ComboBox {
                id: buildTypeCombo
                Kirigami.FormData.label: qsTr("Build Type:")
                Layout.fillWidth: true
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: qsTr("NAND Image"), value: "nand" },
                    { text: qsTr("XeLL Image"), value: "xell" },
                    { text: qsTr("Donor"), value: "donor" }
                ]
            }

            QQC2.ComboBox {
                id: xellHackCombo
                Kirigami.FormData.label: qsTr("Hack:")
                Layout.fillWidth: true
                visible: root.isXeLL
                model: BuilderController.xellHacks
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        BuilderController.setSelectedXellHack(value);
                    }
                }
            }

            QQC2.ComboBox {
                id: xellImageCombo
                Kirigami.FormData.label: qsTr("Image:")
                Layout.fillWidth: true
                visible: root.isXeLL
                model: BuilderController.xellImages
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        BuilderController.setSelectedXellImage(value);
                    }
                }
            }

            QQC2.ComboBox {
                id: versionCombo
                Kirigami.FormData.label: qsTr("Version:")
                Layout.fillWidth: true
                visible: !root.isXeLL
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
                visible: !root.isXeLL
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
                visible: !root.isXeLL
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
                visible: !root.isXeLL
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
                visible: !root.isXeLL && versionCombo.currentText !== "" && imageTypeCombo.currentText !== "" && consoleCombo.currentText !== ""

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
                        "mode": "advanced",
                        "buildType": root.buildType,
                        "xellOnly": root.isXeLL,
                        "donorMode": root.isDonor,
                        "version": root.isXeLL ? "" : root.version,
                        "imageType": root.isXeLL ? "" : root.imageType,
                        "console": root.isXeLL ? "" : root.consoleModel,
                        "smc": root.isXeLL ? "" : root.smcFile,
                        "hack": root.isXeLL ? root.xellHack : "",
                        "image": root.isXeLL ? root.xellImage : "",
                        "patches": root.isXeLL ? [] : root.activePatches,
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
