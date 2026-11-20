import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.gxoss.genexis
import "../components"

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Bad Update")

    property alias entry: entryCombo.currentText
    property alias payload: payloadCombo.currentText
    property var activePatches: []

    signal saveRequested(url fileUrl, var config)

    PatchesDialog {
        id: patchesDialog
        availablePatches: ["nohdmiwait", "USBdsec"]
        selectedPatches: root.activePatches
        onPatchesAccepted: patches => root.activePatches = patches
    }

    FileDialog {
        id: saveFileDialog
        title: qsTr("Save As")
        fileMode: FileDialog.SaveFile
        onAccepted: {
            var config = {
                "entry": root.entry,
                "payload": root.payload,
                "patches": root.activePatches
            };
            root.saveRequested(selectedFile, config);
        }
    }

    Kirigami.FormLayout {
        QQC2.ComboBox {
            id: entryCombo
            Kirigami.FormData.label: qsTr("Entry:")
            Layout.fillWidth: true
            model: ["Rock Band Blitz", "Avatar"]
        }

        QQC2.ComboBox {
            id: payloadCombo
            Kirigami.FormData.label: qsTr("Payload:")
            Layout.fillWidth: true
            model: ["FreeMyXe", "XeUnshackle"]
        }

        RowLayout {
            Kirigami.FormData.label: qsTr("Patches:")
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

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
                id: saveAsButton
                text: qsTr("Save As…")
                icon.name: "system-run"
                highlighted: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 16
                onClicked: saveFileDialog.open()
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Connections {
        target: BadUpdateController
        ignoreUnknownSignals: true
        function onErrorOccurred(errorMessage) {
            var appWin = root.Window ? root.Window.window : null;
            if (appWin && typeof appWin.showError === "function") {
                appWin.showError(qsTr("BadUpdate Error"), errorMessage);
            }
        }
    }
}
