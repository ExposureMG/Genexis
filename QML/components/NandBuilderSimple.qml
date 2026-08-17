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

    signal buildRequested(var config)

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

            QQC2.ComboBox {
                id: buildVersionCombo
                Kirigami.FormData.label: qsTr("Build Version:")
                Layout.fillWidth: true
                visible: buildTypeCombo.currentText !== qsTr("XeLL Image")
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.simpleVersions : ["Latest"]
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedSimpleVersion(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: imageTypeCombo
                Kirigami.FormData.label: qsTr("Image Type:")
                Layout.fillWidth: true
                visible: buildTypeCombo.currentText !== qsTr("XeLL Image")
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.simpleImageTypes : ["Retail", "FreeBoot", "Devkit"]
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedSimpleImageType(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: hackVersionCombo
                Kirigami.FormData.label: qsTr("Hack Version:")
                Layout.fillWidth: true
                visible: buildTypeCombo.currentText !== qsTr("XeLL Image")
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.simpleHacks : ["RGH 3"]
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedSimpleHack(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: xellHackCombo
                Kirigami.FormData.label: qsTr("Hack:")
                Layout.fillWidth: true
                visible: buildTypeCombo.currentText === qsTr("XeLL Image")
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.xellHacks : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedXellHack(currentText);
                    }
                }
            }

            QQC2.ComboBox {
                id: xellImageCombo
                Kirigami.FormData.label: qsTr("Image:")
                Layout.fillWidth: true
                visible: buildTypeCombo.currentText === qsTr("XeLL Image")
                model: typeof nandBuilderController !== "undefined" ? nandBuilderController.xellImages : []
                onCurrentTextChanged: {
                    if (typeof nandBuilderController !== "undefined" && currentText !== "") {
                        nandBuilderController.setSelectedXellImage(currentText);
                    }
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("Patches:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                visible: buildTypeCombo.currentText !== qsTr("XeLL Image") && buildVersionCombo.currentText !== "" && imageTypeCombo.currentText !== ""

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

        Item {
            Layout.fillHeight: true
        }

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
                    "imageType": root.buildType === qsTr("XeLL Image") ? "" : root.imageType,
                    "hackVersion": root.buildType === qsTr("XeLL Image") ? "" : root.hackVersion,
                    "patches": root.activePatches
                };
                root.buildRequested(config);
            }
        }
    }
}
