import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    property alias buildType: buildTypeCombo.currentText
    property alias buildVersion: buildVersionCombo.currentText
    property alias imageType: imageTypeCombo.currentText
    property alias hackVersion: hackVersionCombo.currentText
    property var activePatches: []

    readonly property bool isXeLL: buildTypeCombo.currentText === qsTr("XeLL Image")

    signal buildRequested(var config)

    PatchesDialog {
        id: patchesDialog
        availablePatches: nandBuilderController.availablePatches
        activePatches: root.activePatches
        onPatchesSaved: patches => root.activePatches = patches
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
                model: [qsTr("NAND Image"), qsTr("XeLL Image")]
            }

            SelectorCombo {
                id: buildVersionCombo
                Kirigami.FormData.label: qsTr("Build Version:")
                visible: !root.isXeLL
                model: nandBuilderController.simpleVersions
                onSelected: value => nandBuilderController.setSelectedSimpleVersion(value)
            }

            SelectorCombo {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                visible: !root.isXeLL
                model: nandBuilderController.simpleImageTypes
                onSelected: value => nandBuilderController.setSelectedSimpleImageType(value)
            }

            SelectorCombo {
                id: hackVersionCombo
                Kirigami.FormData.label: qsTr("Hack Version:")
                visible: !root.isXeLL
                model: nandBuilderController.simpleHacks
                onSelected: value => nandBuilderController.setSelectedSimpleHack(value)
            }

            SelectorCombo {
                Kirigami.FormData.label: qsTr("Hack:")
                visible: root.isXeLL
                model: nandBuilderController.xellHacks
                onSelected: value => nandBuilderController.setSelectedXellHack(value)
            }

            SelectorCombo {
                Kirigami.FormData.label: qsTr("Image:")
                visible: root.isXeLL
                model: nandBuilderController.xellImages
                onSelected: value => nandBuilderController.setSelectedXellImage(value)
            }

            PatchesRow {
                Kirigami.FormData.label: qsTr("Patches:")
                visible: !root.isXeLL && buildVersionCombo.currentText !== "" && imageTypeCombo.currentText !== ""
                activePatches: root.activePatches
                onPatchesClicked: patchesDialog.open()
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
                    "buildVersion": root.isXeLL ? "" : root.buildVersion,
                    "imageType": root.isXeLL ? "" : root.imageType,
                    "hackVersion": root.isXeLL ? "" : root.hackVersion,
                    "patches": root.activePatches
                };
                root.buildRequested(config);
            }
        }
    }
}
