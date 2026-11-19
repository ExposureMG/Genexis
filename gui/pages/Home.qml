import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.Page {
    id: root
    title: qsTr("Home")

    FileDialog {
        id: homeFilePicker
        title: qsTr("Open NAND / Image File")
        onAccepted: {
            NandSession.openFile(selectedFile, "");
        }
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
                        visible: !NandSession.isLoading
                        source: {
                            var path = NandSession.loadedFilePath;
                            var ext = path.split('.').pop().toLowerCase();
                            if (ext === "svf" || ext === "xsvf")
                                return "cpu";
                            return path !== "" ? "drive-harddisk" : "drive-harddisk-symbolic";
                        }
                    }

                    Image {
                        anchors.fill: parent
                        visible: NandSession.isLoading
                        source: "qrc:/qt/qml/org/gxoss/genexis/gui/assets/icons/loading-gear.svg"
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
                            if (NandSession.isLoading) {
                                return qsTr("Loading NAND Image…");
                            }
                            if (NandSession.isNandLoaded) {
                                var titleStr = NandSession.consoleTarget !== "" ? NandSession.consoleTarget : qsTr("Target NAND Image");
                                if (NandSession.imageSize !== "") {
                                    titleStr += " (" + NandSession.imageSize + ")";
                                }
                                return titleStr;
                            }
                            var path = NandSession.loadedFilePath;
                            return path !== "" ? qsTr("Target NAND Image") : qsTr("No Image Loaded");
                        }
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                    }

                    QQC2.Label {
                        text: {
                            if (NandSession.isLoading) {
                                return qsTr("Parsing NAND header and metadata…");
                            }
                            var path = NandSession.loadedFilePath;
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
                    enabled: !NandSession.isLoading
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
