import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    property alias buildType: buildTypeCombo.currentText

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

    readonly property bool isXeLL: buildTypeCombo.currentText === qsTr("XeLL Image")

    PatchesDialog {
        id: patchesDialog
        availablePatches: nandBuilderController.availablePatches
        activePatches: root.activePatches
        onPatchesSaved: patches => root.activePatches = patches
    }

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
            tempCpuKey = root.advancedOptions.cpuKey || "";
            tempKvPath = root.advancedOptions.kvPath || "";
            tempSmcPath = root.advancedOptions.smcPath || "";
            tempCustomArgs = root.advancedOptions.customArgs || "";
        }

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: optionsDialog.width - optionsDialog.padding * 2

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
                    };
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

            QQC2.ComboBox {
                id: buildTypeCombo
                Kirigami.FormData.label: qsTr("Build Type:")
                Layout.fillWidth: true
                model: [qsTr("NAND Image"), qsTr("XeLL Image"), qsTr("Donor")]
            }

            SelectorCombo {
                id: xellHackCombo
                Kirigami.FormData.label: qsTr("Hack:")
                visible: root.isXeLL
                model: nandBuilderController.xellHacks
                onSelected: value => nandBuilderController.setSelectedXellHack(value)
            }

            SelectorCombo {
                id: xellImageCombo
                Kirigami.FormData.label: qsTr("Image:")
                visible: root.isXeLL
                model: nandBuilderController.xellImages
                onSelected: value => nandBuilderController.setSelectedXellImage(value)
            }

            SelectorCombo {
                id: versionCombo
                Kirigami.FormData.label: qsTr("Version:")
                visible: !root.isXeLL
                model: nandBuilderController.buildVersions
                onSelected: value => nandBuilderController.setSelectedVersion(value)
            }

            SelectorCombo {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                visible: !root.isXeLL
                model: nandBuilderController.imageTypes
                onSelected: value => nandBuilderController.setSelectedImageType(value)
            }

            SelectorCombo {
                id: consoleCombo
                Kirigami.FormData.label: qsTr("Console:")
                visible: !root.isXeLL
                model: nandBuilderController.consoles
                onSelected: value => nandBuilderController.setSelectedConsole(value)
            }

            SelectorCombo {
                id: smcCombo
                Kirigami.FormData.label: qsTr("SMC:")
                visible: !root.isXeLL
                model: nandBuilderController.smcFiles
                onSelected: value => nandBuilderController.setSelectedSmc(value)
            }

            PatchesRow {
                Kirigami.FormData.label: qsTr("Patches & Options:")
                spacing: Kirigami.Units.mediumSpacing
                visible: !root.isXeLL && versionCombo.currentText !== "" && imageTypeCombo.currentText !== "" && consoleCombo.currentText !== ""
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
                    "buildType": root.buildType,
                    "xellOnly": root.isXeLL,
                    "donorMode": root.buildType === qsTr("Donor"),
                    "patches": root.activePatches,
                    "options": root.advancedOptions
                };
                if (root.isXeLL) {
                    config["hack"] = root.xellHack;
                    config["image"] = root.xellImage;
                } else {
                    config["version"] = root.version;
                    config["imageType"] = root.imageType;
                    config["console"] = root.consoleModel;
                    config["smc"] = root.smcFile;
                }
                root.buildRequested(config);
            }
        }
    }
}
