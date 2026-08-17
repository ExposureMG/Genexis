import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

QQC2.ToolBar {
    id: root

    property string pageTitle: qsTr("Genexis")

    signal menuRequested
    signal fileOpened(string filePath, string key)

    position: QQC2.ToolBar.Header
    Kirigami.Dialog {
        id: openFileDialog
        title: qsTr("Open File")
        standardButtons: QQC2.Dialog.NoButton
        padding: Kirigami.Units.largeSpacing

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 30

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: filePathField
                    Layout.fillWidth: true
                    placeholderText: qsTr("File path…")
                    readOnly: true
                    onTextChanged: {
                        if (text !== "" && typeof nandController !== "undefined") {
                            var autoKey = nandController.detectCpuKey(text);
                            if (autoKey !== "") {
                                keyField.text = autoKey;
                            }
                        }
                    }
                }

                QQC2.Button {
                    icon.name: "document-open"
                    text: qsTr("Browse…")
                    onClicked: filePicker.open()
                }
            }

            QQC2.TextField {
                id: keyField
                Layout.fillWidth: true
                placeholderText: qsTr("Key")
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing

                Item {
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    text: qsTr("Cancel")
                    onClicked: openFileDialog.close()
                }

                QQC2.Button {
                    text: qsTr("Confirm")
                    highlighted: true
                    enabled: filePathField.text !== "" && !(typeof nandController !== "undefined" && nandController.isLoading)
                    onClicked: {
                        root.fileOpened(filePathField.text, keyField.text);
                        openFileDialog.close();
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }

    FileDialog {
        id: filePicker
        onAccepted: {
            filePathField.text = selectedFile;
            if (typeof nandController !== "undefined") {
                var autoKey = nandController.detectCpuKey(selectedFile);
                if (autoKey !== "") {
                    keyField.text = autoKey;
                }
            }
        }
    }

    contentItem: RowLayout {
        spacing: 0

        QQC2.ToolButton {
            id: menuButton
            icon.name: "application-menu"
            display: QQC2.AbstractButton.IconOnly
            onClicked: root.menuRequested()

            QQC2.ToolTip.text: qsTr("Navigation menu")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }

        QQC2.Label {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            text: root.pageTitle
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        RowLayout {
            visible: typeof nandController !== "undefined" && nandController.isLoading
            spacing: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing

            Image {
                source: "qrc:/qt/qml/org/gxoss/genexis/assets/loading-gear.svg"
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                fillMode: Image.PreserveAspectFit

                NumberAnimation on rotation {
                    from: 0
                    to: 360
                    duration: 1000
                    loops: Animation.Infinite
                    running: visible
                }
            }

            QQC2.Label {
                text: qsTr("Loading…")
                font.bold: true
                color: Kirigami.Theme.highlightColor
            }
        }

        QQC2.ToolButton {
            text: qsTr("Open File")
            icon.name: "document-open"
            display: QQC2.AbstractButton.TextBesideIcon
            Layout.rightMargin: Kirigami.Units.largeSpacing
            enabled: !(typeof nandController !== "undefined" && nandController.isLoading)
            onClicked: openFileDialog.open()
        }
    }
}
