import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Settings")

    property alias buildBackend: buildBackendCombo.currentText
    property alias flashBackend: flashBackendCombo.currentText
    property alias timingFlashBackend: timingBackendCombo.currentText
    property alias wirelessBackend: wirelessBackendCombo.currentText

    signal loadRequested
    signal saveRequested(var config)

    function selectBackend(combo, name) {
        combo.currentIndex = Math.max(0, combo.model.indexOf(name));
    }

    function reloadBackends() {
        settingsController.loadSettings();
        root.selectBackend(buildBackendCombo, settingsController.buildBackend);
        root.selectBackend(flashBackendCombo, settingsController.flashBackend);
        root.selectBackend(timingBackendCombo, settingsController.timingFlashBackend);
        root.selectBackend(wirelessBackendCombo, settingsController.wirelessBackend);
    }

    Component.onCompleted: root.reloadBackends()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.ComboBox {
                id: buildBackendCombo
                Kirigami.FormData.label: qsTr("Build Backend:")
                Layout.fillWidth: true
                model: settingsController.availableBuildBackends
            }

            QQC2.ComboBox {
                id: flashBackendCombo
                Kirigami.FormData.label: qsTr("Flash Backend:")
                Layout.fillWidth: true
                model: settingsController.availableFlashBackends
            }

            QQC2.ComboBox {
                id: timingBackendCombo
                Kirigami.FormData.label: qsTr("Timing Flash Backend:")
                Layout.fillWidth: true
                model: settingsController.availableTimingFlashBackends
            }

            QQC2.ComboBox {
                id: wirelessBackendCombo
                Kirigami.FormData.label: qsTr("Wireless Backend:")
                Layout.fillWidth: true
                model: settingsController.availableWirelessBackends
            }
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 18
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.mediumSpacing

            QQC2.Button {
                id: loadButton
                text: qsTr("Load")
                icon.name: "document-open"
                Layout.fillWidth: true
                onClicked: {
                    root.reloadBackends();
                    root.loadRequested();
                }
            }

            QQC2.Button {
                id: saveButton
                text: qsTr("Save")
                icon.name: "document-save"
                highlighted: true
                Layout.fillWidth: true
                onClicked: {
                    settingsController.buildBackend = root.buildBackend;
                    settingsController.flashBackend = root.flashBackend;
                    settingsController.timingFlashBackend = root.timingFlashBackend;
                    settingsController.wirelessBackend = root.wirelessBackend;
                    settingsController.saveSettings();
                    var config = {
                        "buildBackend": root.buildBackend,
                        "flashBackend": root.flashBackend,
                        "timingFlashBackend": root.timingFlashBackend,
                        "wirelessBackend": root.wirelessBackend
                    };
                    root.saveRequested(config);
                }
            }
        }
    }
}
