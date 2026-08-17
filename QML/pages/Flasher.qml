import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Flasher")

    property string selectedFilePath: typeof nandController !== "undefined" ? nandController.loadedFilePath : ""
    property alias operation: operationCombo.currentText
    property string logText: qsTr("[INFO] Flasher initialized.\n[INFO] Ready for operation.\n")

    function getActionButtonText() {
        var ext = root.selectedFilePath.split('.').pop().toLowerCase();
        var isJtag = (ext === "svf" || ext === "xsvf");

        if (operationCombo.currentText === qsTr("Detect")) {
            return isJtag ? qsTr("Detect JTAG Chain") : qsTr("Detect Flash Config");
        } else if (operationCombo.currentText === qsTr("Read")) {
            return qsTr("Read NAND");
        } else if (operationCombo.currentText === qsTr("Erase")) {
            return qsTr("Erase NAND");
        } else if (operationCombo.currentText === qsTr("Write")) {
            return isJtag ? qsTr("Flash CPLD") : qsTr("Write NAND");
        }
        return qsTr("Execute");
    }

    QQC2.Dialog {
        id: timingWizardDialog
        title: qsTr("Timing Wizard")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 22
        implicitHeight: Kirigami.Units.gridUnit * 12

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.mediumSpacing
            QQC2.Label {
                text: qsTr("CPLD Timing Wizard stub")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: timingWizardDialog.close()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        
        Kirigami.AbstractCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            contentItem: Item {
                implicitHeight: Kirigami.Units.gridUnit * 9
                Layout.fillWidth: true

                Image {
                    anchors.centerIn: parent
                    width: Math.min(parent.width, Kirigami.Units.gridUnit * 14)
                    height: parent.height
                    source: (typeof flasherController !== "undefined") ? flasherController.connectedFlasherImage : "qrc:/qt/qml/org/gxoss/genexis/assets/noflasher.png"
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
                model: (typeof flasherController !== "undefined") ? flasherController.availableFlashers : ["None", "UpdClient (Network)"]
                currentIndex: (typeof flasherController !== "undefined") ? model.indexOf(flasherController.selectedFlasher) : 0
                onCurrentTextChanged: {
                    if (typeof flasherController !== "undefined" && currentText !== "") {
                        flasherController.setSelectedFlasher(currentText);
                    }
                }
            }

            
            RowLayout {
                id: ipRow
                Kirigami.FormData.label: qsTr("IP:")
                Layout.fillWidth: true
                visible: typeof flasherController !== "undefined" && flasherController.isUpdClientSelected
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: ipField
                    Layout.fillWidth: true
                    placeholderText: qsTr("192.168.1.xxx")
                    text: (typeof flasherController !== "undefined") ? flasherController.targetIp : ""
                    onTextChanged: {
                        if (typeof flasherController !== "undefined") {
                            flasherController.setTargetIp(text);
                        }
                    }
                }

                QQC2.Button {
                    text: qsTr("Search")
                    icon.name: "system-search"
                    onClicked: {
                        if (typeof flasherController !== "undefined") {
                            flasherController.searchNetworkDevices();
                        }
                    }
                }
            }

            QQC2.ComboBox {
                id: deviceNetworkCombo
                Kirigami.FormData.label: qsTr("Device:")
                Layout.fillWidth: true
                visible: typeof flasherController !== "undefined" && flasherController.isUpdClientSelected
                model: (typeof flasherController !== "undefined") ? flasherController.detectedNetworkDevices : []
                onCurrentTextChanged: {
                    if (currentText !== "") {
                        var ipPart = currentText.split(' ')[0];
                        if (typeof flasherController !== "undefined") {
                            flasherController.setTargetIp(ipPart);
                        }
                    }
                }
            }

            QQC2.TextField {
                id: detectedField
                Kirigami.FormData.label: qsTr("Detected:")
                Layout.fillWidth: true
                readOnly: true
                enabled: typeof flasherController === "undefined" || !flasherController.isUpdClientSelected
                text: (typeof flasherController !== "undefined") ? flasherController.detectedHardwareInfo : qsTr("000000")
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
                model: [qsTr("Read"), qsTr("Erase"), qsTr("Write"), qsTr("Detect")]
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

        Item {
            Layout.fillHeight: true
        }

        Connections {
            target: typeof flasherController !== "undefined" ? flasherController : null
            ignoreUnknownSignals: true
            function onLogOutput(message) {
                root.logText += message + "\n";
                var appWin = root.Window ? root.Window.window : null;
                if (appWin && typeof appWin.appendConsoleLog === "function") {
                    appWin.appendConsoleLog(message);
                }
            }
            function onProgressUpdated(progress, status) {
                var appWin = root.Window ? root.Window.window : null;
                if (appWin && typeof appWin.updateConsoleProgress === "function") {
                    appWin.updateConsoleProgress(Math.round(progress * 100), status);
                }
            }
            function onOperationFinished(success, message) {
                var appWin = root.Window ? root.Window.window : null;
                if (appWin && typeof appWin.appendConsoleLog === "function") {
                    appWin.appendConsoleLog(message);
                }
            }
        }

        QQC2.Button {
            id: actionButton
            text: root.getActionButtonText()
            icon.name: "system-run"
            highlighted: true
            enabled: typeof flasherController === "undefined" || !flasherController.isBusy
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 18
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                var op = operationCombo.currentText;
                var isWrite = (op === qsTr("Write"));
                var ext = root.selectedFilePath.split('.').pop().toLowerCase();
                var isJtag = (ext === "svf" || ext === "xsvf");

                if ((isWrite || isJtag) && op !== qsTr("Detect") && op !== qsTr("Read") && op !== qsTr("Erase") && root.selectedFilePath === "") {
                    var appWin = root.Window ? root.Window.window : null;
                    if (appWin && typeof appWin.showError === "function") {
                        appWin.showError(qsTr("Flasher Error"), qsTr("Please select a target file before performing this operation."));
                    }
                    return;
                }

                if (op === qsTr("Detect")) {
                    if (typeof flasherController !== "undefined") {
                        flasherController.detectHardware(root.selectedFilePath);
                    }
                    return;
                }

                var timestamp = new Date().toLocaleTimeString();
                var initLog = "[" + timestamp + "] Initiating " + op + " operation...\n";
                if (root.selectedFilePath !== "") {
                    initLog += "[" + timestamp + "] Target file: " + root.selectedFilePath + "\n";
                }

                root.logText += initLog;

                var appWin = root.Window ? root.Window.window : null;
                if (appWin && typeof appWin.showConsole === "function") {
                    appWin.showConsole(qsTr("Flasher Console"), initLog, true);
                }

                if (typeof flasherController !== "undefined") {
                    flasherController.performOperation(root.selectedFilePath, op, {});
                }
            }
        }
    }
}
