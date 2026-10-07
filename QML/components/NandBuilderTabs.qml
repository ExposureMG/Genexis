import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    property int currentTabIndex: 0
    property string buildLogText: ""

    Connections {
        target: nandBuilderController

        function onBuildStarted() {
            var timestamp = new Date().toLocaleTimeString();
            var initMsg = qsTr("[%1] Initiating NAND build operation...").arg(timestamp) + "\n";
            root.buildLogText = initMsg;
            var appWin = root.Window ? root.Window.window : null;
            if (appWin && typeof appWin.showConsole === "function") {
                appWin.showConsole(qsTr("NAND Builder Output"), initMsg, true);
            }
        }

        function onBuildProgress(percentage, status) {
            var line = "[" + percentage + "%] " + status + "\n";
            root.buildLogText += line;
            var appWin = root.Window ? root.Window.window : null;
            if (appWin) {
                if (typeof appWin.updateConsoleProgress === "function") {
                    appWin.updateConsoleProgress(percentage, status);
                }
                if (typeof appWin.appendConsoleLog === "function") {
                    appWin.appendConsoleLog("[" + percentage + "%] " + status);
                }
            }
        }

        function onBuildFinished(success, outputPath, logOutput) {
            var endLog = "";
            if (logOutput && logOutput.length > 0) {
                endLog += "\n" + qsTr("--- Builder Log Output ---") + "\n" + logOutput + "\n";
            }
            if (success) {
                endLog += "\n" + qsTr("[SUCCESS] NAND Image created successfully: %1").arg(outputPath) + "\n";
            } else {
                endLog += "\n" + qsTr("[ERROR] NAND Image creation failed.") + "\n";
            }
            root.buildLogText += endLog;

            var appWin = root.Window ? root.Window.window : null;
            if (appWin) {
                if (typeof appWin.appendConsoleLog === "function") {
                    appWin.appendConsoleLog(endLog);
                }
                if (typeof appWin.updateConsoleProgress === "function") {
                    appWin.updateConsoleProgress(100, success ? qsTr("Build Complete!") : qsTr("Build Failed."));
                }
                if (!success && typeof appWin.showError === "function") {
                    appWin.showError(qsTr("NAND Build Error"), logOutput || qsTr("NAND Image creation failed."));
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing

            Item {
                Layout.fillWidth: true
            }

            QQC2.TabBar {
                id: navTabBar
                currentIndex: root.currentTabIndex
                onCurrentIndexChanged: root.currentTabIndex = currentIndex

                QQC2.TabButton {
                    text: qsTr("Simple")
                }
                QQC2.TabButton {
                    text: qsTr("Advanced")
                }
                QQC2.TabButton {
                    text: qsTr("Donor")
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Loader {
            id: builderLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                if (root.currentTabIndex === 1)
                    return advancedComponent;
                if (root.currentTabIndex === 2)
                    return donorComponent;
                return simpleComponent;
            }
        }
    }

    Connections {
        target: builderLoader.item

        function onBuildRequested(config) {
            nandBuilderController.buildImage(config);
        }
    }

    Component {
        id: simpleComponent
        NandBuilderSimple {}
    }

    Component {
        id: advancedComponent
        NandBuilderAdvanced {}
    }

    Component {
        id: donorComponent
        NandBuilderDonor {}
    }
}
