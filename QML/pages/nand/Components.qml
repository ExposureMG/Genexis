import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Components")

    // Signal to request switching to Keyvault tab
    signal keyvaultRequested

    enabled: typeof nandController !== "undefined" && nandController.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

    // Component Metadata Properties bound to nandController
    property string smcConsole: typeof nandController !== "undefined" ? nandController.consoleTarget : ""
    property string smcVersion: typeof nandController !== "undefined" ? nandController.smcVersion : ""
    property string smcType: typeof nandController !== "undefined" ? nandController.smcType : ""

    property string cbVersion: typeof nandController !== "undefined" ? nandController.cbVersion : ""
    property string cbSize: typeof nandController !== "undefined" ? nandController.cbSize : ""
    property string cbMagic: typeof nandController !== "undefined" ? nandController.cbMagic : ""
    property string cbLdv: typeof nandController !== "undefined" ? nandController.cbLdv : ""
    property string cbPairing: typeof nandController !== "undefined" ? nandController.cbPairing : ""

    property string cbAVersion: typeof nandController !== "undefined" ? nandController.cbAVersion : ""
    property string cbALdv: typeof nandController !== "undefined" ? nandController.cbALdv : ""
    property string cbAPairing: typeof nandController !== "undefined" ? nandController.cbAPairing : ""

    property string cbBVersion: typeof nandController !== "undefined" ? nandController.cbBVersion : ""

    property string cdVersion: typeof nandController !== "undefined" ? nandController.cdVersion : ""
    property string ceVersion: typeof nandController !== "undefined" ? nandController.ceVersion : ""
    property string xellVersion: "0.99"
    property string xellType: "XeLL Reloaded"

    property string cf0Version: typeof nandController !== "undefined" ? nandController.cf0Version : ""
    property string cg0Version: typeof nandController !== "undefined" ? nandController.cg0Version : ""
    property string cf0Ldv: typeof nandController !== "undefined" ? nandController.cf0Ldv : ""
    property string cf0Pairing: typeof nandController !== "undefined" ? nandController.cf0Pairing : ""

    property string cf1Version: typeof nandController !== "undefined" ? nandController.cf1Version : ""
    property string cg1Version: typeof nandController !== "undefined" ? nandController.cg1Version : ""
    property string cf1Ldv: typeof nandController !== "undefined" ? nandController.cf1Ldv : ""
    property string cf1Pairing: typeof nandController !== "undefined" ? nandController.cf1Pairing : ""

    // Keyvault Properties
    property string kvConsoleModel: typeof nandController !== "undefined" ? nandController.consoleTarget : ""
    property string kvType: typeof nandController !== "undefined" ? nandController.consoleType : ""
    property string kvSerial: typeof nandController !== "undefined" ? nandController.serialNumber : ""
    property string kvConsoleId: typeof nandController !== "undefined" ? nandController.consoleId : ""
    property string kvDvdKey: typeof nandController !== "undefined" ? nandController.dvdKey : ""
    property string kvRegion: typeof nandController !== "undefined" ? nandController.gameRegion : ""

    //  1. SMC Details Dialog
    QQC2.Dialog {
        id: smcDialog
        title: qsTr("SMC Firmware Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 24

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: smcForm.implicitHeight

            Kirigami.FormLayout {
                id: smcForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Target:")
                    text: root.smcConsole
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.smcVersion
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Type:")
                    text: root.smcType
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: smcDialog.close()
            }
        }
    }

    //  2. CB Details Dialog
    QQC2.Dialog {
        id: cbDialog
        title: qsTr("CB Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 26

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: cbForm.implicitHeight

            Kirigami.FormLayout {
                id: cbForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.cbVersion
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Size:")
                    text: root.cbSize !== "" ? root.cbSize + " bytes" : ""
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Magic:")
                    text: root.cbMagic
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("LDV:")
                    text: root.cbLdv
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("PD:")
                    text: root.cbPairing
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: cbDialog.close()
            }
        }
    }

    //  3. CB_A Details Dialog
    QQC2.Dialog {
        id: cbADialog
        title: qsTr("CB_A Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 26

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: cbAForm.implicitHeight

            Kirigami.FormLayout {
                id: cbAForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.cbAVersion
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("LDV:")
                    text: root.cbALdv
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("PD:")
                    text: root.cbAPairing
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: cbADialog.close()
            }
        }
    }

    //  4. CB_B Details Dialog
    QQC2.Dialog {
        id: cbBDialog
        title: qsTr("CB_B Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 24

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: cbBForm.implicitHeight

            Kirigami.FormLayout {
                id: cbBForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.cbBVersion
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: cbBDialog.close()
            }
        }
    }

    //  5. CD Details Dialog
    QQC2.Dialog {
        id: cdDialog
        title: qsTr("CD Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 24

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: cdForm.implicitHeight

            Kirigami.FormLayout {
                id: cdForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.cdVersion
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: cdDialog.close()
            }
        }
    }

    //  6. CE Details Dialog
    QQC2.Dialog {
        id: ceDialog
        title: qsTr("CE Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 24

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: ceForm.implicitHeight

            Kirigami.FormLayout {
                id: ceForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.ceVersion
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.bold: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: ceDialog.close()
            }
        }
    }

    //  7. XeLL Details Dialog
    QQC2.Dialog {
        id: xellDialog
        title: qsTr("XeLL Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 24

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: xellForm.implicitHeight

            Kirigami.FormLayout {
                id: xellForm
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Version:")
                    text: root.xellVersion
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("Variant:")
                    text: root.xellType
                    readOnly: true
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: xellDialog.close()
            }
        }
    }

    //  8. Patchslot 0 Details Dialog
    QQC2.Dialog {
        id: patch0Dialog
        title: qsTr("Patchslot 0 Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 26

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: patch0Form.implicitHeight

            Kirigami.FormLayout {
                id: patch0Form
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CF Version:")
                    text: root.cf0Version
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CG Version:")
                    text: root.cg0Version
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CF LDV:")
                    text: root.cf0Ldv
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CF PD:")
                    text: root.cf0Pairing
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: patch0Dialog.close()
            }
        }
    }

    //  9. Patchslot 1 Details Dialog
    QQC2.Dialog {
        id: patch1Dialog
        title: qsTr("Patchslot 1 Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        padding: Kirigami.Units.largeSpacing
        implicitWidth: Kirigami.Units.gridUnit * 26

        contentItem: QQC2.ScrollView {
            clip: true
            implicitHeight: patch1Form.implicitHeight

            Kirigami.FormLayout {
                id: patch1Form
                wideMode: true
                width: parent.width

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CF Version:")
                    text: root.cf1Version
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CG Version:")
                    text: root.cg1Version
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CF LDV:")
                    text: root.cf1Ldv
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                QQC2.TextField {
                    Kirigami.FormData.label: qsTr("CF PD:")
                    text: root.cf1Pairing
                    placeholderText: qsTr("Empty")
                    readOnly: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button {
                text: qsTr("Close")
                onClicked: patch1Dialog.close()
            }
        }
    }

    //  Placeholder Label when no components are loaded
    QQC2.Label {
        anchors.centerIn: parent
        visible: cardsView.count === 0
        text: root.enabled ? qsTr("No components discovered in this NAND image.") : qsTr("No components discovered. Load a NAND image to view components.")
        font.bold: true
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
        color: Kirigami.Theme.disabledTextColor
    }

    //  Main CardsListView with Dynamic Cards Model
    Kirigami.CardsListView {
        id: cardsView
        anchors.fill: parent
        model: typeof nandController !== "undefined" ? nandController.components : []

        delegate: DelegateChooser {
            role: "cardType"

            // 1. SMC Firmware Choice
            DelegateChoice {
                roleValue: "smc"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: smcLayout.implicitWidth
                        implicitHeight: smcLayout.implicitHeight

                        ColumnLayout {
                            id: smcLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("SMC Firmware")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.smcVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: smcDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 2. Generic CB Choice
            DelegateChoice {
                roleValue: "cb"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: cbLayout.implicitWidth
                        implicitHeight: cbLayout.implicitHeight

                        ColumnLayout {
                            id: cbLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("CB (2BL)")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.cbVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: cbDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 3. CB_A Choice
            DelegateChoice {
                roleValue: "cb_a"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: cbALayout.implicitWidth
                        implicitHeight: cbALayout.implicitHeight

                        ColumnLayout {
                            id: cbALayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("CB_A (2BL)")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.cbAVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: cbADialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 4. CB_B Choice
            DelegateChoice {
                roleValue: "cb_b"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: cbBLayout.implicitWidth
                        implicitHeight: cbBLayout.implicitHeight

                        ColumnLayout {
                            id: cbBLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("CB_B (2BL)")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.cbBVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: cbBDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 5. CD Choice
            DelegateChoice {
                roleValue: "cd"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: cdLayout.implicitWidth
                        implicitHeight: cdLayout.implicitHeight

                        ColumnLayout {
                            id: cdLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("CD (4BL)")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.cdVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: cdDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 6. CE Choice (Kernel)
            DelegateChoice {
                roleValue: "ce"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: ceLayout.implicitWidth
                        implicitHeight: ceLayout.implicitHeight

                        ColumnLayout {
                            id: ceLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("CE (5BL / Kernel)")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.ceVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: ceDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 7. XeLL Choice
            DelegateChoice {
                roleValue: "xell"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: xellLayout.implicitWidth
                        implicitHeight: xellLayout.implicitHeight

                        ColumnLayout {
                            id: xellLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("XeLL")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.xellVersion)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: xellDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 8. Patchslot 0 Choice
            DelegateChoice {
                roleValue: "patch0"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: patch0Layout.implicitWidth
                        implicitHeight: patch0Layout.implicitHeight

                        ColumnLayout {
                            id: patch0Layout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("Patchslot 0")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.cf0Version)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: patch0Dialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 9. Patchslot 1 Choice
            DelegateChoice {
                roleValue: "patch1"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: patch1Layout.implicitWidth
                        implicitHeight: patch1Layout.implicitHeight

                        ColumnLayout {
                            id: patch1Layout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("Patchslot 1")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Version: %1").arg(modelData.versionStr ? modelData.versionStr : root.cf1Version)
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: patch1Dialog.open()
                                }
                            }
                        }
                    }
                }
            }

            // 10. Keyvault Choice
            DelegateChoice {
                roleValue: "keyvault"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: keyvaultCardLayout.implicitWidth
                        implicitHeight: keyvaultCardLayout.implicitHeight

                        ColumnLayout {
                            id: keyvaultCardLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                            }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading {
                                level: 2
                                text: qsTr("Keyvault")
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label {
                                    text: qsTr("Serial: %1").arg(modelData.versionStr ? modelData.versionStr : (root.kvSerial !== "" ? root.kvSerial : qsTr("Encrypted")))
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                QQC2.Button {
                                    text: qsTr("Details…")
                                    icon.name: "dialog-information"
                                    onClicked: root.keyvaultRequested()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
