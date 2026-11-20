import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Flasher")

    property string selectedFilePath: NandSession.loadedFilePath
    property alias operation: operationCombo.currentValue
    property string logText: qsTr("[INFO] Flasher initialized.\n[INFO] Ready for operation.\n")

    function getActionButtonText() {
        var ext = root.selectedFilePath.split('.').pop().toLowerCase();
        var isJtag = (ext === "svf" || ext === "xsvf");

        switch (root.operation) {
        case "detect":
            return isJtag ? qsTr("Detect JTAG Chain") : qsTr("Detect Flash Config");
        case "read":
            return qsTr("Read NAND");
        case "erase":
            return qsTr("Erase NAND");
        case "write":
            return isJtag ? qsTr("Flash CPLD") : qsTr("Write NAND");
        default:
            return qsTr("Execute");
        }
    }

    Kirigami.Dialog {
        id: timingWizardDialog
        title: qsTr("Timing Wizard")
        preferredWidth: Kirigami.Units.gridUnit * 22
        padding: Kirigami.Units.largeSpacing
        standardButtons: Kirigami.Dialog.Close

        QQC2.Label {
            text: qsTr("CPLD Timing Wizard stub")
            wrapMode: Text.WordWrap
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: Item {
                implicitHeight: Kirigami.Units.gridUnit * 9

                Image {
                    anchors.centerIn: parent
                    width: Math.min(parent.width, Kirigami.Units.gridUnit * 14)
                    height: parent.height
                    source: FlasherController.connectedFlasherImage || "qrc:/qt/qml/org/gxoss/genexis/gui/assets/vendor/noflasher.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                }
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.ComboBox {
                id: flasherCombo
                Kirigami.FormData.label: qsTr("Flasher:")
                Layout.fillWidth: true
                model: FlasherController.availableFlashers
                currentIndex: Math.max(0, indexOfValue(FlasherController.selectedFlasher))
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        FlasherController.setSelectedFlasher(value);
                    }
                }
            }

            RowLayout {
                id: ipRow
                Kirigami.FormData.label: qsTr("IP:")
                Layout.fillWidth: true
                visible: FlasherController.isUpdClientSelected
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: ipField
                    Layout.fillWidth: true
                    placeholderText: qsTr("192.168.1.xxx")
                    text: FlasherController.targetIp
                    onTextEdited: FlasherController.setTargetIp(text)
                }

                QQC2.Button {
                    text: qsTr("Search")
                    icon.name: "system-search"
                    onClicked: FlasherController.searchNetworkDevices()
                }
            }

            QQC2.ComboBox {
                id: deviceNetworkCombo
                Kirigami.FormData.label: qsTr("Device:")
                Layout.fillWidth: true
                visible: FlasherController.isUpdClientSelected
                model: FlasherController.detectedNetworkDevices
                onActivated: index => {
                    var value = textAt(index);
                    if (value !== "") {
                        FlasherController.setTargetIp(value.split(' ')[0]);
                    }
                }
            }

            QQC2.TextField {
                id: detectedField
                Kirigami.FormData.label: qsTr("Detected:")
                Layout.fillWidth: true
                readOnly: true
                enabled: !FlasherController.isUpdClientSelected
                text: FlasherController.detectedHardwareInfo
            }

            QQC2.TextField {
                id: selectedFileField
                Kirigami.FormData.label: qsTr("Selected File:")
                Layout.fillWidth: true
                readOnly: true
                text: root.selectedFilePath !== "" ? root.selectedFilePath : qsTr("None")
            }

            QQC2.ComboBox {
                id: operationCombo
                Kirigami.FormData.label: qsTr("Operation:")
                Layout.fillWidth: true
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: qsTr("Read"), value: "read" },
                    { text: qsTr("Erase"), value: "erase" },
                    { text: qsTr("Write"), value: "write" },
                    { text: qsTr("Detect"), value: "detect" }
                ]
            }

            QQC2.Button {
                id: timingWizardButton
                Kirigami.FormData.label: qsTr("Tools:")
                text: qsTr("Timing Wizard")
                icon.name: "preferences-system-time"
                Layout.fillWidth: true
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                onClicked: timingWizardDialog.open()
            }
        }
    }

    Connections {
        target: FlasherController
        ignoreUnknownSignals: true
        function onLogOutput(message) {
            root.logText += message + "\n";
            var appWin = root.Window ? root.Window.window : null;
            if (appWin && typeof appWin.appendConsoleLog === "function") {
                appWin.appendConsoleLog(message);
            }
        }
        function onProgressUpdated(percentage, status) {
            var appWin = root.Window ? root.Window.window : null;
            if (appWin && typeof appWin.updateConsoleProgress === "function") {
                appWin.updateConsoleProgress(percentage, status);
            }
        }
        function onOperationFinished(success, message) {
            var appWin = root.Window ? root.Window.window : null;
            if (appWin && typeof appWin.appendConsoleLog === "function") {
                appWin.appendConsoleLog(message);
            }
        }
    }

    footer: QQC2.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: actionButton
                text: root.getActionButtonText()
                icon.name: "system-run"
                highlighted: true
                enabled: !FlasherController.isBusy
                Layout.preferredWidth: Kirigami.Units.gridUnit * 18

                onClicked: {
                    var op = root.operation;

                    if (op === "write" && root.selectedFilePath === "") {
                        var appWin = root.Window ? root.Window.window : null;
                        if (appWin && typeof appWin.showError === "function") {
                            appWin.showError(qsTr("Flasher Error"), qsTr("Please select a target file before performing this operation."));
                        }
                        return;
                    }

                    if (op === "detect") {
                        FlasherController.detectHardware(root.selectedFilePath);
                        return;
                    }

                    var timestamp = new Date().toLocaleTimeString();
                    var initLog = "[" + timestamp + "] Initiating " + operationCombo.currentText + " operation...\n";
                    if (root.selectedFilePath !== "") {
                        initLog += "[" + timestamp + "] Target file: " + root.selectedFilePath + "\n";
                    }

                    root.logText += initLog;

                    var appWin = root.Window ? root.Window.window : null;
                    if (appWin && typeof appWin.showConsole === "function") {
                        appWin.showConsole(qsTr("Flasher Console"), initLog, true);
                    }

                    FlasherController.performOperation(root.selectedFilePath, op, {});
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
