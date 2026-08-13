import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: root

    title: qsTr("Keyvault Details")

    // Properties for data binding
    property string consoleModel: qsTr("Trinity")
    property string kvType: qsTr("Type 2 (Slim)")
    property string consoleId: "028045612398"
    property string serialNumber: "504938210305"
    property string region: "0x0069 (PAL-EUR)"
    property string osig: "PLDS    DG-16D4S        0501"
    property string dvdKey: "A1B2C3D4E5F67890123456789ABCDEF0"
    property string mfrDate: "2011-05-18"
    property bool isHashed: true
    property bool fcrtRequired: true

    QQC2.ToolTip {
        id: copyToolTip
        timeout: 2000
    }

    function copyToClipboard(text, fieldName, buttonItem) {
        clipboardHelper.text = text;
        clipboardHelper.selectAll();
        clipboardHelper.copy();
        clipboardHelper.deselect();

        copyToolTip.text = qsTr("%1 copied to clipboard!").arg(fieldName);
        copyToolTip.x = buttonItem.width / 2 - copyToolTip.width / 2;
        copyToolTip.y = -copyToolTip.height - Kirigami.Units.smallSpacing;
        copyToolTip.open();
    }

    // Hidden text field used to invoke standard QML clipboard copy
    QQC2.TextField {
        id: clipboardHelper
        visible: false
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        implicitWidth: Math.min(root.parent ? root.parent.width * 0.9 : 600, Kirigami.Units.gridUnit * 35)

        // Header Status Badges
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            // Hashed Status Badge
            Rectangle {
                implicitWidth: hashedLabel.implicitWidth + Kirigami.Units.largeSpacing
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                radius: Kirigami.Units.smallSpacing
                color: root.isHashed ? Qt.rgba(0.1, 0.7, 0.3, 0.15) : Qt.rgba(0.9, 0.5, 0.1, 0.15)
                border.color: root.isHashed ? Qt.rgba(0.1, 0.7, 0.3, 0.6) : Qt.rgba(0.9, 0.5, 0.1, 0.6)

                RowLayout {
                    anchors.centerIn: parent
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: root.isHashed ? "security-high" : "security-low"
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }

                    QQC2.Label {
                        id: hashedLabel
                        text: root.isHashed ? qsTr("Hashed KV") : qsTr("Unhashed KV")
                        font.bold: true
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }
            }

            // FCRT Status Badge
            Rectangle {
                implicitWidth: fcrtLabel.implicitWidth + Kirigami.Units.largeSpacing
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                radius: Kirigami.Units.smallSpacing
                color: root.fcrtRequired ? Qt.rgba(0.8, 0.2, 0.2, 0.15) : Qt.rgba(0.2, 0.6, 0.9, 0.15)
                border.color: root.fcrtRequired ? Qt.rgba(0.8, 0.2, 0.2, 0.6) : Qt.rgba(0.2, 0.6, 0.9, 0.6)

                RowLayout {
                    anchors.centerIn: parent
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: root.fcrtRequired ? "dialog-warning" : "dialog-information"
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }

                    QQC2.Label {
                        id: fcrtLabel
                        text: root.fcrtRequired ? qsTr("FCRT Required") : qsTr("FCRT Not Required")
                        font.bold: true
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Details Form Layout
        Kirigami.FormLayout {
            Layout.fillWidth: true

            // 1. Console Model
            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Console Model:")
                text: root.consoleModel
                readOnly: true
                Layout.fillWidth: true
            }

            // 2. Keyvault Type
            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Keyvault Type:")
                text: root.kvType
                readOnly: true
                Layout.fillWidth: true
            }

            // 3. Serial Number
            RowLayout {
                Kirigami.FormData.label: qsTr("Serial Number:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: root.serialNumber
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copySerialBtn
                    icon.name: "edit-copy"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.serialNumber, qsTr("Serial Number"), copySerialBtn)
                    QQC2.ToolTip.text: qsTr("Copy Serial Number")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // 4. Console ID
            RowLayout {
                Kirigami.FormData.label: qsTr("Console ID:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: root.consoleId
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copyConsoleIdBtn
                    icon.name: "edit-copy"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.consoleId, qsTr("Console ID"), copyConsoleIdBtn)
                    QQC2.ToolTip.text: qsTr("Copy Console ID")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // 5. DVD Key
            RowLayout {
                Kirigami.FormData.label: qsTr("DVD Key:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: root.dvdKey
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copyDvdKeyBtn
                    icon.name: "edit-copy"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.dvdKey, qsTr("DVD Key"), copyDvdKeyBtn)
                    QQC2.ToolTip.text: qsTr("Copy DVD Key")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // 6. OSIG String
            RowLayout {
                Kirigami.FormData.label: qsTr("OSIG String:")
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: root.osig
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    id: copyOsigBtn
                    icon.name: "edit-copy"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.osig, qsTr("OSIG String"), copyOsigBtn)
                    QQC2.ToolTip.text: qsTr("Copy OSIG")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // 7. Game Region
            QQC2.TextField {
                Kirigami.FormData.label: qsTr("Game Region:")
                text: root.region
                readOnly: true
                Layout.fillWidth: true
            }

            // 8. Manufacturing Date
            QQC2.TextField {
                Kirigami.FormData.label: qsTr("MFR Date:")
                text: root.mfrDate
                readOnly: true
                Layout.fillWidth: true
            }
        }

        // Close Action
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                text: qsTr("Close")
                icon.name: "dialog-close"
                onClicked: root.close()
            }
        }
    }
}
