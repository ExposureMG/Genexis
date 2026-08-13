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

            // Serial Number
            RowLayout {
                Kirigami.FormData.label: qsTr("Serial Number:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: typeof nandController !== "undefined" ? nandController.serialNumber : ""
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
                    onClicked: root.copyToClipboard(nandController.serialNumber, qsTr("Serial Number"), copySerialBtn)
                    QQC2.ToolTip.text: qsTr("Copy Serial Number")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // Console ID
            RowLayout {
                Kirigami.FormData.label: qsTr("Console ID:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: typeof nandController !== "undefined" ? nandController.consoleId : ""
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
                    onClicked: root.copyToClipboard(nandController.consoleId, qsTr("Console ID"), copyConsoleIdBtn)
                    QQC2.ToolTip.text: qsTr("Copy Console ID")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // DVD Key
            RowLayout {
                Kirigami.FormData.label: qsTr("DVD Key:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: typeof nandController !== "undefined" ? nandController.dvdKey : ""
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
                    onClicked: root.copyToClipboard(nandController.dvdKey, qsTr("DVD Key"), copyDvdKeyBtn)
                    QQC2.ToolTip.text: qsTr("Copy DVD Key")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // Game Region
            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Game Region:")
                text: typeof nandController !== "undefined" ? nandController.gameRegion : ""
                placeholderText: qsTr("Encrypted")
                readOnly: true
                Layout.fillWidth: true
            }

            // Console Type
            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Console Type:")
                text: typeof nandController !== "undefined" ? nandController.consoleType : ""
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
