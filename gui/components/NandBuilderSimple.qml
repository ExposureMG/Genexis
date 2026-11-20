import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    property alias buildType: buildTypeCombo.currentValue
    property alias buildVersion: buildVersionCombo.currentText
    property alias imageType: imageTypeCombo.currentText
    property alias hackVersion: hackVersionCombo.currentText
    property var activePatches: []

    readonly property bool isXeLL: buildTypeCombo.currentValue === "xell"

    signal buildRequested(var config)

    PatchesDialog {
        id: patchesDialog
        availablePatches: BuilderController.availablePatches
        selectedPatches: root.activePatches
        onPatchesAccepted: patches => root.activePatches = patches
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
                { text: qsTr("XeLL Image"), value: "xell" }
            ]
        }

        QQC2.ComboBox {
            id: buildVersionCombo
            Kirigami.FormData.label: qsTr("Build Version:")
            Layout.fillWidth: true
            visible: !root.isXeLL
            model: BuilderController.simpleVersions
            onActivated: index => {
                var value = textAt(index);
                if (value !== "") {
                    BuilderController.setSelectedSimpleVersion(value);
                }
            }
        }

        QQC2.ComboBox {
            id: imageTypeCombo
            Kirigami.FormData.label: qsTr("Image Type:")
            Layout.fillWidth: true
            visible: !root.isXeLL
            model: BuilderController.simpleImageTypes
            onActivated: index => {
                var value = textAt(index);
                if (value !== "") {
                    BuilderController.setSelectedSimpleImageType(value);
                }
            }
        }

        QQC2.ComboBox {
            id: hackVersionCombo
            Kirigami.FormData.label: qsTr("Hack Version:")
            Layout.fillWidth: true
            visible: !root.isXeLL
            model: BuilderController.simpleHacks
            onActivated: index => {
                var value = textAt(index);
                if (value !== "") {
                    BuilderController.setSelectedSimpleHack(value);
                }
            }
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

        RowLayout {
            Kirigami.FormData.label: qsTr("Patches:")
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: !root.isXeLL && buildVersionCombo.currentText !== "" && imageTypeCombo.currentText !== ""

            QQC2.Button {
                id: patchesButton
                text: root.activePatches.length > 0 ? qsTr("Patches (%1 selected)").arg(root.activePatches.length) : qsTr("Patches")
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
                        "mode": "simple",
                        "buildType": root.buildType,
                        "xellOnly": root.isXeLL,
                        "donorMode": false,
                        "version": root.isXeLL ? "" : root.buildVersion,
                        "imageType": root.isXeLL ? "" : root.imageType,
                        "hack": root.isXeLL ? xellHackCombo.currentText : root.hackVersion,
                        "image": root.isXeLL ? xellImageCombo.currentText : "",
                        "patches": root.isXeLL ? [] : root.activePatches,
                        "options": ({})
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
