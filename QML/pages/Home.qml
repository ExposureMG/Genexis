import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import "../components/nand"

Kirigami.Page {
    id: root
    title: qsTr("Home")

    NandMetadata {
        id: nand
    }

    FileDialog {
        id: homeFilePicker
        title: qsTr("Open NAND / Image File")
        onAccepted: nandController.openFile(selectedFile, "")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Item {
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium

                    Kirigami.Icon {
                        anchors.fill: parent
                        visible: !nandController.isLoading
                        source: {
                            var path = nandController.loadedFilePath;
                            var ext = path.split('.').pop().toLowerCase();
                            if (ext === "svf" || ext === "xsvf")
                                return "cpu";
                            return path !== "" ? "drive-harddisk" : "drive-harddisk-symbolic";
                        }
                    }

                    Image {
                        anchors.fill: parent
                        visible: nandController.isLoading
                        source: "qrc:/qt/qml/org/gxoss/genexis/assets/loading-gear.svg"
                        fillMode: Image.PreserveAspectFit

                        NumberAnimation on rotation {
                            from: 0
                            to: 360
                            duration: 1000
                            loops: Animation.Infinite
                            running: visible
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing / 2

                    QQC2.Label {
                        text: {
                            if (nandController.isLoading) {
                                return qsTr("Loading NAND Image…");
                            }
                            if (nandController.isNandLoaded) {
                                var titleStr = nand.consoleTarget !== "" ? nand.consoleTarget : qsTr("Target NAND Image");
                                if (nand.imageSize !== "") {
                                    titleStr += " (" + nand.imageSize + ")";
                                }
                                return titleStr;
                            }
                            return nandController.loadedFilePath !== "" ? qsTr("Target NAND Image") : qsTr("No Image Loaded");
                        }
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                    }

                    QQC2.Label {
                        text: {
                            if (nandController.isLoading) {
                                return qsTr("Parsing NAND header and metadata…");
                            }
                            var path = nandController.loadedFilePath;
                            return path !== "" ? path : qsTr("Click 'Open File' in statusbar or browse to load a NAND image");
                        }
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        color: Kirigami.Theme.disabledTextColor
                    }
                }

                QQC2.Button {
                    icon.name: "document-open"
                    text: qsTr("Browse…")
                    enabled: !nandController.isLoading
                    onClicked: {
                        var appWin = root.Window ? root.Window.window : null;
                        if (appWin && typeof appWin.openOpenFileDialog === "function") {
                            appWin.openOpenFileDialog();
                        } else if (typeof statusBar !== "undefined" && typeof statusBar.openOpenFileDialog === "function") {
                            statusBar.openOpenFileDialog();
                        } else {
                            homeFilePicker.open();
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
