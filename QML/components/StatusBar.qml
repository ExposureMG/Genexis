import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

QQC2.ToolBar {
    id: root

    // Current page title — bound from Main.qml
    property string pageTitle: qsTr("Genexis")

    // Emitted when the hamburger button is clicked
    signal menuRequested()

    // Emitted when the user confirms the open-file dialog
    signal fileOpened(string filePath, string key)

    position: QQC2.ToolBar.Header

    // ── Open File Dialog ─────────────────────────────────────────────────
    Kirigami.Dialog {
        id: openFileDialog
        title: qsTr("Open File")
        standardButtons: QQC2.Dialog.NoButton
        padding: Kirigami.Units.largeSpacing

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 30

            // Row 1: file path textbox + browse button
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
                            var autoKey = nandController.detectCpuKey(text)
                            if (autoKey !== "") {
                                keyField.text = autoKey
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

            // Row 2: key input
            QQC2.TextField {
                id: keyField
                Layout.fillWidth: true
                placeholderText: qsTr("Key")
            }

            // Row 3: cancel + confirm, centre-aligned
            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing

                Item { Layout.fillWidth: true }

                QQC2.Button {
                    text: qsTr("Cancel")
                    onClicked: openFileDialog.close()
                }

                QQC2.Button {
                    text: qsTr("Confirm")
                    highlighted: true
                    enabled: filePathField.text !== ""
                    onClicked: {
                        root.fileOpened(filePathField.text, keyField.text)
                        openFileDialog.close()
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }
    }

    // Native OS file picker — writes chosen path back into the text field
    FileDialog {
        id: filePicker
        onAccepted: {
            filePathField.text = selectedFile
            if (typeof nandController !== "undefined") {
                var autoKey = nandController.detectCpuKey(selectedFile)
                if (autoKey !== "") {
                    keyField.text = autoKey
                }
            }
        }
    }

    contentItem: RowLayout {
        spacing: 0

        // ── Hamburger / menu toggle ──────────────────────────────────────
        QQC2.ToolButton {
            id: menuButton
            icon.name: "application-menu"
            display: QQC2.AbstractButton.IconOnly
            onClicked: root.menuRequested()

            QQC2.ToolTip.text: qsTr("Navigation menu")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        // ── Divider ──────────────────────────────────────────────────────
        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }

        // ── Page title ───────────────────────────────────────────────────
        QQC2.Label {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            text: root.pageTitle
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        // ── Open File action ─────────────────────────────────────────────
        QQC2.ToolButton {
            text: qsTr("Open File")
            icon.name: "document-open"
            display: QQC2.AbstractButton.TextBesideIcon
            Layout.rightMargin: Kirigami.Units.largeSpacing
            onClicked: openFileDialog.open()
        }
    }
}
