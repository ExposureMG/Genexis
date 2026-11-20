import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Keyvault")

    readonly property bool decrypted: NandSession.isNandLoaded && NandSession.isCpuKeyLoaded

    function copyToClipboard(text, fieldName) {
        clipboardHelper.text = text;
        clipboardHelper.selectAll();
        clipboardHelper.copy();
        clipboardHelper.deselect();

        var appWin = root.Window ? root.Window.window : null;
        if (appWin && typeof appWin.showPassiveNotification === "function") {
            appWin.showPassiveNotification(qsTr("%1 copied to clipboard").arg(fieldName));
        }
    }

    QQC2.TextField {
        id: clipboardHelper
        visible: false
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        visible: !root.decrypted
        icon.name: "lock"
        text: qsTr("Keyvault Encrypted")
        explanation: qsTr("Load a NAND image with a valid CPU key to decrypt the keyvault.")
    }

    Kirigami.FormLayout {
        visible: root.decrypted

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
                icon.name: "edit-copy-symbolic"
                display: QQC2.AbstractButton.IconOnly
                onClicked: root.copyToClipboard(InfoController.serialNumber, qsTr("Serial Number"))
                QQC2.ToolTip.text: qsTr("Copy Serial Number")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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
                icon.name: "edit-copy-symbolic"
                display: QQC2.AbstractButton.IconOnly
                onClicked: root.copyToClipboard(InfoController.consoleId, qsTr("Console ID"))
                QQC2.ToolTip.text: qsTr("Copy Console ID")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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
                icon.name: "edit-copy-symbolic"
                display: QQC2.AbstractButton.IconOnly
                onClicked: root.copyToClipboard(InfoController.dvdKey, qsTr("DVD Key"))
                QQC2.ToolTip.text: qsTr("Copy DVD Key")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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
}
