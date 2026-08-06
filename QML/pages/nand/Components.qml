import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Components")

    // Signal to request switching to Keyvault tab
    signal keyvaultRequested()

    // Component Metadata Properties
    property string smcConsole: "Trinity"
    property string smcVersion: "3.1"
    property string smcType: "Clean / Glitch"

    property string cbVersion: "9231"
    property string cbPairing: "0x00000000"
    property string cbLockdown: "0x0000000F"

    property string cbAVersion: "9231"
    property string cbAPairing: "0x00000000"
    property string cbALockdown: "0x0000000F"

    property string cbBVersion: "9231"

    property string cdVersion: "8453"
    property string ceVersion: "17559"
    property string xellVersion: "0.99"
    property string xellType: "XeLL Reloaded"

    property string cf0Version: "17559"
    property string cf0Pairing: "000000"
    property string cf0Lockdown: "12"
    property string cg0Version: "17559"

    property string cf1Version: "17559"
    property string cf1Pairing: "000000"
    property string cf1Lockdown: "12"
    property string cg1Version: "17559"

    // Keyvault Properties
    property string kvConsoleModel: "Trinity"
    property string kvType: "Type 2 (Slim)"
    property string kvSerial: "504938210305"
    property string kvConsoleId: "028045612398"
    property string kvDvdKey: "A1B2C3D4E5F67890123456789ABCDEF0"
    property string kvOsig: "PLDS    DG-16D4S        0501"
    property string kvRegion: "0x0069 (PAL-EUR)"
    property string kvMfrDate: "2011-05-18"
    property bool kvHashed: true
    property bool kvFcrt: true

    // ── 1. SMC Details Dialog (1/5th bigger: 24 x 17 grid units) ─────────
    QQC2.Dialog {
        id: smcDialog
        title: qsTr("SMC Firmware Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 17

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: smcDialog.width - smcDialog.padding * 2

                QQC2.Label { text: qsTr("Console Target:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.smcConsole; readOnly: true; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("SMC Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.smcVersion; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("SMC Type:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.smcType; readOnly: true; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: smcDialog.close() }
        }
    }

    // ── 2. CB Details Dialog (26 x 19 grid units) ───────────────────────
    QQC2.Dialog {
        id: cbDialog
        title: qsTr("CB Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 26
        implicitHeight: Kirigami.Units.gridUnit * 19

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: cbDialog.width - cbDialog.padding * 2

                QQC2.Label { text: qsTr("CB Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbVersion; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("Pairing Data:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbPairing; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("Lockdown Value:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbLockdown; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: cbDialog.close() }
        }
    }

    // ── 3. CB_A Details Dialog (26 x 19 grid units) ─────────────────────
    QQC2.Dialog {
        id: cbADialog
        title: qsTr("CB_A Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 26
        implicitHeight: Kirigami.Units.gridUnit * 19

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: cbADialog.width - cbADialog.padding * 2

                QQC2.Label { text: qsTr("CB_A Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbAVersion; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CB_A Pairing:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbAPairing; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CB_A Lockdown:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbALockdown; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: cbADialog.close() }
        }
    }

    // ── 4. CB_B Details Dialog (24 x 15 grid units) ─────────────────────
    QQC2.Dialog {
        id: cbBDialog
        title: qsTr("CB_B Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 15

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: cbBDialog.width - cbBDialog.padding * 2

                QQC2.Label { text: qsTr("CB_B Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cbBVersion; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: cbBDialog.close() }
        }
    }

    // ── 5. CD Details Dialog (24 x 15 grid units) ───────────────────────
    QQC2.Dialog {
        id: cdDialog
        title: qsTr("CD Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 15

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: cdDialog.width - cdDialog.padding * 2

                QQC2.Label { text: qsTr("CD Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cdVersion; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: cdDialog.close() }
        }
    }

    // ── 6. CE Details Dialog (24 x 15 grid units) ───────────────────────
    QQC2.Dialog {
        id: ceDialog
        title: qsTr("CE (Kernel) Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 15

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: ceDialog.width - ceDialog.padding * 2

                QQC2.Label { text: qsTr("Kernel Version (CE):"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.ceVersion; readOnly: true; font.bold: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: ceDialog.close() }
        }
    }

    // ── 7. XeLL Details Dialog (24 x 17 grid units) ──────────────────────
    QQC2.Dialog {
        id: xellDialog
        title: qsTr("XeLL Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 17

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: xellDialog.width - xellDialog.padding * 2

                QQC2.Label { text: qsTr("XeLL Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.xellVersion; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("XeLL Variant:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.xellType; readOnly: true; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: xellDialog.close() }
        }
    }

    // ── 8. Patchslot 0 Details Dialog (26 x 20 grid units) ─────────────
    QQC2.Dialog {
        id: patch0Dialog
        title: qsTr("Patchslot 0 Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 26
        implicitHeight: Kirigami.Units.gridUnit * 20

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: patch0Dialog.width - patch0Dialog.padding * 2

                QQC2.Label { text: qsTr("CF0 Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cf0Version; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CF0 Pairing:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cf0Pairing; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CF0 Lockdown:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cf0Lockdown; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CG0 Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cg0Version; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: patch0Dialog.close() }
        }
    }

    // ── 9. Patchslot 1 Details Dialog (26 x 20 grid units) ─────────────
    QQC2.Dialog {
        id: patch1Dialog
        title: qsTr("Patchslot 1 Details")
        modal: true
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        implicitWidth: Kirigami.Units.gridUnit * 26
        implicitHeight: Kirigami.Units.gridUnit * 20

        contentItem: QQC2.ScrollView {
            clip: true
            GridLayout {
                columns: 2
                rowSpacing: Kirigami.Units.mediumSpacing
                columnSpacing: Kirigami.Units.largeSpacing
                width: patch1Dialog.width - patch1Dialog.padding * 2

                QQC2.Label { text: qsTr("CF1 Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cf1Version; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CF1 Pairing:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cf1Pairing; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CF1 Lockdown:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cf1Lockdown; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }

                QQC2.Label { text: qsTr("CG1 Version:"); font.bold: true; color: Kirigami.Theme.disabledTextColor; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                QQC2.TextField { text: root.cg1Version; readOnly: true; font.family: "Monospace"; Layout.fillWidth: true }
            }
        }

        footer: QQC2.DialogButtonBox {
            alignment: Qt.AlignRight
            QQC2.Button { text: qsTr("Close"); onClicked: patch1Dialog.close() }
        }
    }

    // ── Model for CardsListView (Notice CB is NOT in default model) ──────
    ListModel {
        id: cardsModel

        ListElement { cardType: "smc" }
        ListElement { cardType: "cb_a" }
        ListElement { cardType: "cb_b" }
        ListElement { cardType: "cd" }
        ListElement { cardType: "ce" }
        ListElement { cardType: "xell" }
        ListElement { cardType: "patch0" }
        ListElement { cardType: "patch1" }
        ListElement { cardType: "keyvault" }
    }

    // ── Main CardsListView with Predefined DelegateChooser Choices ──────
    Kirigami.CardsListView {
        id: cardsView
        anchors.fill: parent
        model: cardsModel

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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("SMC Firmware") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.smcVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: smcDialog.open() }
                            }
                        }
                    }
                }
            }

            // 2. Generic CB Choice (Defined for CB, not pushed to cardsModel by default)
            DelegateChoice {
                roleValue: "cb"
                Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: cbLayout.implicitWidth
                        implicitHeight: cbLayout.implicitHeight

                        ColumnLayout {
                            id: cbLayout
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("CB") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.cbVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: cbDialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("CB_A") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.cbAVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: cbADialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("CB_B") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.cbBVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: cbBDialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("CD") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.cdVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: cdDialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("CE (Kernel)") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.ceVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: ceDialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("XeLL") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.xellVersion); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: xellDialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("Patchslot 0") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.cf0Version); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: patch0Dialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("Patchslot 1") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Version: %1").arg(root.cf1Version); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: patch1Dialog.open() }
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
                            anchors { left: parent.left; top: parent.top; right: parent.right }
                            spacing: Kirigami.Units.mediumSpacing

                            Kirigami.Heading { level: 2; text: qsTr("Keyvault") }
                            Kirigami.Separator { Layout.fillWidth: true }

                            RowLayout {
                                Layout.fillWidth: true
                                QQC2.Label { text: qsTr("Type: %1").arg(root.kvType); font.bold: true; Layout.fillWidth: true }
                                QQC2.Button { text: qsTr("Details…"); icon.name: "dialog-information"; onClicked: root.keyvaultRequested() }
                            }
                        }
                    }
                }
            }
        }
    }
}
