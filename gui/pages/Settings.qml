import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Settings")

    property alias buildBackend: buildBackendCombo.currentText
    property alias flashBackend: flashBackendCombo.currentText
    property alias timingFlashBackend: timingBackendCombo.currentText
    property alias wirelessBackend: wirelessBackendCombo.currentText

    signal loadRequested
    signal saveRequested(var config)

    function syncFromController() {
        SettingsController.loadSettings();
        buildBackendCombo.currentIndex = Math.max(0, buildBackendCombo.indexOfValue(SettingsController.buildBackend));
        flashBackendCombo.currentIndex = Math.max(0, flashBackendCombo.indexOfValue(SettingsController.flashBackend));
        timingBackendCombo.currentIndex = Math.max(0, timingBackendCombo.indexOfValue(SettingsController.timingFlashBackend));
        wirelessBackendCombo.currentIndex = Math.max(0, wirelessBackendCombo.indexOfValue(SettingsController.wirelessBackend));
    }

    Component.onCompleted: syncFromController()

    Kirigami.FormLayout {
        QQC2.ComboBox {
            id: buildBackendCombo
            Kirigami.FormData.label: qsTr("Build Backend:")
            Layout.fillWidth: true
            model: SettingsController.availableBuildBackends
        }

        QQC2.ComboBox {
            id: flashBackendCombo
            Kirigami.FormData.label: qsTr("Flash Backend:")
            Layout.fillWidth: true
            model: SettingsController.availableFlashBackends
        }

        QQC2.ComboBox {
            id: timingBackendCombo
            Kirigami.FormData.label: qsTr("Timing Flash Backend:")
            Layout.fillWidth: true
            model: SettingsController.availableTimingFlashBackends
        }

        QQC2.ComboBox {
            id: wirelessBackendCombo
            Kirigami.FormData.label: qsTr("Wireless Backend:")
            Layout.fillWidth: true
            model: SettingsController.availableWirelessBackends
        }
    }

    footer: QQC2.ToolBar {
        contentItem: RowLayout {
            spacing: Kirigami.Units.mediumSpacing

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: loadButton
                text: qsTr("Load")
                icon.name: "document-open"
                Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                onClicked: {
                    root.syncFromController();
                    root.loadRequested();
                }
            }

            QQC2.Button {
                id: saveButton
                text: qsTr("Save")
                icon.name: "document-save"
                highlighted: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                onClicked: {
                    SettingsController.buildBackend = root.buildBackend;
                    SettingsController.flashBackend = root.flashBackend;
                    SettingsController.timingFlashBackend = root.timingFlashBackend;
                    SettingsController.wirelessBackend = root.wirelessBackend;
                    SettingsController.saveSettings();
                    var config = {
                        "buildBackend": root.buildBackend,
                        "flashBackend": root.flashBackend,
                        "timingFlashBackend": root.timingFlashBackend,
                        "wirelessBackend": root.wirelessBackend
                    };
                    root.saveRequested(config);
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
