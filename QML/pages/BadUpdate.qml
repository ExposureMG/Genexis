import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import "../components"

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Bad Update")

    property alias entry: entryCombo.currentText
    property alias payload: payloadCombo.currentText
    property var activePatches: []

    signal saveRequested(string filePath, var config)

    PatchesDialog {
        id: patchesDialog
        availablePatches: ["nohdmiwait", "USBdsec"]
        activePatches: root.activePatches
        onPatchesSaved: patches => root.activePatches = patches
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
            console.log("BadUpdate Save As requested for file:", selectedFile, JSON.stringify(config));
            root.saveRequested(selectedFile, config);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

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

            PatchesRow {
                Kirigami.FormData.label: qsTr("Patches:")
                activePatches: root.activePatches
                onPatchesClicked: patchesDialog.open()
            }
        }

        Item {
            Layout.fillHeight: true
        }

        QQC2.Button {
            id: saveAsButton
            text: qsTr("Save As…")
            icon.name: "system-run"
            highlighted: true
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter
            onClicked: saveFileDialog.open()
        }
    }

    Connections {
        target: typeof badUpdateController !== "undefined" ? badUpdateController : null
        ignoreUnknownSignals: true
        function onErrorOccurred(errorMessage) {
            var appWin = root.Window ? root.Window.window : null;
            if (appWin && typeof appWin.showError === "function") {
                appWin.showError(qsTr("BadUpdate Error"), errorMessage);
            }
        }
    }
}
