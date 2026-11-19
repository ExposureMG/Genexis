import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Keyvault")

    enabled: NandSession.isNandLoaded && NandSession.isCpuKeyLoaded
    opacity: enabled ? 1.0 : 0.45

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

                QQC2.TextField {
                    text: InfoController.serialNumber
                    placeholderText: qsTr("Encrypted")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copySerialBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(InfoController.serialNumber, qsTr("Serial Number"), copySerialBtn)
                    QQC2.ToolTip.text: qsTr("Copy Serial Number")
                    QQC2.ToolTip.visible: hovered
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("Console ID:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: InfoController.consoleId
                    placeholderText: qsTr("Encrypted")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copyConsoleIdBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(InfoController.consoleId, qsTr("Console ID"), copyConsoleIdBtn)
                    QQC2.ToolTip.text: qsTr("Copy Console ID")
                    QQC2.ToolTip.visible: hovered
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("DVD Key:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: InfoController.dvdKey
                    placeholderText: qsTr("Encrypted")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copyDvdKeyBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: root.enabled
                    onClicked: root.copyToClipboard(InfoController.dvdKey, qsTr("DVD Key"), copyDvdKeyBtn)
                    QQC2.ToolTip.text: qsTr("Copy DVD Key")
                    QQC2.ToolTip.visible: hovered
                }
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Game Region:")
                text: InfoController.gameRegion
                placeholderText: qsTr("Encrypted")
                readOnly: true
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Console Type:")
                text: InfoController.consoleType
                placeholderText: qsTr("Encrypted")
                readOnly: true
                Layout.fillWidth: true
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
