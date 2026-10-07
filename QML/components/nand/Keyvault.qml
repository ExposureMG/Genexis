import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Keyvault")

    enabled: nandController.isNandLoaded && nandController.isCpuKeyLoaded
    opacity: enabled ? 1.0 : 0.45

    NandMetadata {
        id: nand
    }

    QQC2.ToolTip {
        id: copyToolTip
        timeout: 2000
    }

    function copyToClipboard(text, fieldName, buttonItem) {
        clipboardHelper.text = text;
        clipboardHelper.selectAll();
        clipboardHelper.copy();
        clipboardHelper.deselect();

        copyToolTip.text = qsTr("%1 copied to clipboard").arg(fieldName);
        copyToolTip.x = buttonItem.width / 2 - copyToolTip.width / 2;
        copyToolTip.y = -copyToolTip.height - Kirigami.Units.smallSpacing;
        copyToolTip.open();
    }

    QQC2.TextField {
        id: clipboardHelper
        visible: false
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: root.enabled ? qsTr("Decrypted Keyvault Information") : qsTr("Keyvault Encrypted (Enter CPU Key to Decrypt)")
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            Layout.alignment: Qt.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            RowLayout {
                Kirigami.FormData.label: qsTr("Serial Number:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                InfoField {
                    text: nand.serialNumber
                    placeholderText: qsTr("Encrypted")
                    font.family: "Monospace"
                }

                QQC2.Button {
                    id: copySerialBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(nand.serialNumber, qsTr("Serial Number"), copySerialBtn)
                    QQC2.ToolTip.text: qsTr("Copy Serial Number")
                    QQC2.ToolTip.visible: hovered
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("Console ID:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                InfoField {
                    text: nand.consoleId
                    placeholderText: qsTr("Encrypted")
                    font.family: "Monospace"
                }

                QQC2.Button {
                    id: copyConsoleIdBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(nand.consoleId, qsTr("Console ID"), copyConsoleIdBtn)
                    QQC2.ToolTip.text: qsTr("Copy Console ID")
                    QQC2.ToolTip.visible: hovered
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("DVD Key:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                InfoField {
                    text: nand.dvdKey
                    placeholderText: qsTr("Encrypted")
                    font.family: "Monospace"
                }

                QQC2.Button {
                    id: copyDvdKeyBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(nand.dvdKey, qsTr("DVD Key"), copyDvdKeyBtn)
                    QQC2.ToolTip.text: qsTr("Copy DVD Key")
                    QQC2.ToolTip.visible: hovered
                }
            }

            InfoField {
                Kirigami.FormData.label: qsTr("Game Region:")
                text: nand.gameRegion
                placeholderText: qsTr("Encrypted")
            }

            InfoField {
                Kirigami.FormData.label: qsTr("Console Type:")
                text: nand.consoleType
                placeholderText: qsTr("Encrypted")
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
