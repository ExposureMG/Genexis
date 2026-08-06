import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Keyvault")

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
            text: qsTr("Keyvault Information")
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

            // Console Model
            QQC2.Label {
                text: qsTr("Console Model:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.consoleModel
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            // Keyvault Type
            QQC2.Label {
                text: qsTr("Keyvault Type:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.kvType
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            // Serial Number
            QQC2.Label {
                text: qsTr("Serial Number:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: root.serialNumber
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 14
                }

                QQC2.Button {
                    id: copySerialBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.serialNumber, qsTr("Serial Number"), copySerialBtn)
                    QQC2.ToolTip.text: qsTr("Copy Serial Number")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // Console ID
            QQC2.Label {
                text: qsTr("Console ID:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: root.consoleId
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 14
                }

                QQC2.Button {
                    id: copyConsoleIdBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.consoleId, qsTr("Console ID"), copyConsoleIdBtn)
                    QQC2.ToolTip.text: qsTr("Copy Console ID")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // DVD Key
            QQC2.Label {
                text: qsTr("DVD Key:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: root.dvdKey
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 20
                }

                QQC2.Button {
                    id: copyDvdKeyBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.dvdKey, qsTr("DVD Key"), copyDvdKeyBtn)
                    QQC2.ToolTip.text: qsTr("Copy DVD Key")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // OSIG String
            QQC2.Label {
                text: qsTr("OSIG String:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                QQC2.TextField {
                    text: root.osig
                    readOnly: true
                    font.family: "Monospace"
                    implicitWidth: Kirigami.Units.gridUnit * 20
                }

                QQC2.Button {
                    id: copyOsigBtn
                    icon.name: "edit-copy-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.copyToClipboard(root.osig, qsTr("OSIG String"), copyOsigBtn)
                    QQC2.ToolTip.text: qsTr("Copy OSIG")
                    QQC2.ToolTip.visible: hovered
                }
            }

            // Game Region
            QQC2.Label {
                text: qsTr("Game Region:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.region
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            // Manufacturing Date
            QQC2.Label {
                text: qsTr("MFR Date:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.mfrDate
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            // Hashed Status
            QQC2.Label {
                text: qsTr("Hashed Status:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.isHashed ? qsTr("Hashed KV") : qsTr("Unhashed KV")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }

            // FCRT Requirement
            QQC2.Label {
                text: qsTr("FCRT Status:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            QQC2.TextField {
                text: root.fcrtRequired ? qsTr("FCRT Required") : qsTr("FCRT Not Required")
                readOnly: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                implicitWidth: Kirigami.Units.gridUnit * 14
            }
        }

        Item { Layout.fillHeight: true }
    }
}
