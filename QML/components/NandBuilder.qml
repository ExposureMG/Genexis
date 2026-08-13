import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    property int currentTabIndex: 0
    property string buildLogText: ""

    //  Controller Signals Connection
    Connections {
        target: typeof nandBuilderController !== "undefined" ? nandBuilderController : null

        function onBuildStarted() {
            var timestamp = new Date().toLocaleTimeString();
            var initMsg = "[" + timestamp + "] Initiating NAND build operation...\n";
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
                endLog += "\n--- Builder Log Output ---\n" + logOutput + "\n";
            }
            if (success) {
                endLog += "\n[SUCCESS] NAND Image created successfully: " + outputPath + "\n";
            } else {
                endLog += "\n[ERROR] NAND Image creation failed.\n";
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

    //  Main Layout
    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Centred TabBar Navigation
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

        // Dynamic view container switching between Simple, Advanced, and Donor components
        Loader {
            id: builderLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                if (root.currentTabIndex === 0)
                    return simpleComponent;
                if (root.currentTabIndex === 1)
                    return advancedComponent;
                if (root.currentTabIndex === 2)
                    return donorComponent;
                return simpleComponent;
            }
        }
    }

    Component {
        id: simpleComponent
        NandBuilderSimple {
            onBuildRequested: function (config) {
                if (typeof nandBuilderController !== "undefined") {
                    nandBuilderController.buildImage(config);
                }
            }
        }
    }

    Component {
        id: advancedComponent
        NandBuilderAdvanced {
            onBuildRequested: function (config) {
                if (typeof nandBuilderController !== "undefined") {
                    nandBuilderController.buildImage(config);
                }
            }
        }
    }

    Component {
        id: donorComponent
        NandBuilderDonor {
            onBuildRequested: function (config) {
                if (typeof nandBuilderController !== "undefined") {
                    nandBuilderController.buildImage(config);
                }
            }
        }
    }
}
