import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Keyvault")

    enabled: typeof nandController !== "undefined" && nandController.isNandLoaded && nandController.isCpuKeyLoaded
    opacity: enabled ? 1.0 : 0.45

    QQC2.ToolTip {
        id: copyToolTip
        timeout: 2000
    }

    function copyToClipboard(text, fieldName, buttonItem) {
        clipboardHelper.text = text
        clipboardHelper.selectAll()
        clipboardHelper.copy()
        clipboardHelper.deselect()

        copyToolTip.text = qsTr("%1 copied to clipboard").arg(fieldName)
        copyToolTip.x = buttonItem.width / 2 - copyToolTip.width / 2
        copyToolTip.y = -copyToolTip.height - Kirigami.Units.smallSpacing
        copyToolTip.open()
    }

    QQC2.TextField {
        id: clipboardHelper
        visible: false
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        QQC2.Label {
            text: enabled ? qsTr("Decrypted Keyvault Information") : qsTr("Keyvault Encrypted (Enter CPU Key to Decrypt)")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.mediumSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter

            // Serial Number
            QQC2.Label {
                text: qsTr("Serial Number:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: typeof nandController !== "undefined" ? nandController.serialNumber : ""
                    placeholderText: qsTr("Encrypted")
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 14
                }

                QQC2.Button {
                    id: copySerialBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(nandController.serialNumber, qsTr("Serial Number"), copySerialBtn)
                    QQC2.ToolTip.text: qsTr("Copy Serial Number")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // Console ID
            QQC2.Label {
                text: qsTr("Console ID:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: typeof nandController !== "undefined" ? nandController.consoleId : ""
                    placeholderText: qsTr("Encrypted")
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 14
                }

                QQC2.Button {
                    id: copyConsoleIdBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(nandController.consoleId, qsTr("Console ID"), copyConsoleIdBtn)
                    QQC2.ToolTip.text: qsTr("Copy Console ID")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // DVD Key
            QQC2.Label {
                text: qsTr("DVD Key:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: typeof nandController !== "undefined" ? nandController.dvdKey : ""
                    placeholderText: qsTr("Encrypted")
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 20
                }

                QQC2.Button {
                    id: copyDvdKeyBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(nandController.dvdKey, qsTr("DVD Key"), copyDvdKeyBtn)
                    QQC2.ToolTip.text: qsTr("Copy DVD Key")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // Game Region
            QQC2.Label {
                text: qsTr("Game Region:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.gameRegion : ""
                placeholderText: qsTr("Encrypted")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            // Console Type
            QQC2.Label {
                text: qsTr("Console Type:")
                font.bold: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: typeof nandController !== "undefined" ? nandController.consoleType : ""
                placeholderText: qsTr("Encrypted")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        Item { Layout.fillHeight: true }
    }
}
