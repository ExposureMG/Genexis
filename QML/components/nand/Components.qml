import QtQuick
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Components")

    signal keyvaultRequested

    enabled: nandController.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

    function versionSummary(versionStr, fallback) {
        return qsTr("Version: %1").arg(versionStr ? versionStr : fallback);
    }

    NandMetadata {
        id: nand
    }

    ComponentDetailsDialog {
        id: smcDialog
        title: qsTr("SMC Firmware Details")

        InfoField {
            Kirigami.FormData.label: qsTr("Target:")
            text: nand.consoleTarget
        }
        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            text: nand.smcVersion
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("Type:")
            text: nand.smcType
        }
    }

    ComponentDetailsDialog {
        id: cbDialog
        title: qsTr("CB Details")
        implicitWidth: Kirigami.Units.gridUnit * 26

        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            text: nand.cbVersion
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("Size:")
            text: nand.cbSize !== "" ? qsTr("%1 bytes").arg(nand.cbSize) : ""
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("Magic:")
            text: nand.cbMagic
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("LDV:")
            text: nand.cbLdv
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("PD:")
            text: nand.cbPairing
            font.family: "Monospace"
        }
    }

    ComponentDetailsDialog {
        id: cbADialog
        title: qsTr("CB_A Details")
        implicitWidth: Kirigami.Units.gridUnit * 26

        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            text: nand.cbAVersion
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("LDV:")
            text: nand.cbALdv
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("PD:")
            text: nand.cbAPairing
            font.family: "Monospace"
        }
    }

    ComponentDetailsDialog {
        id: cbBDialog
        title: qsTr("CB_B Details")

        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            text: nand.cbBVersion
            font.family: "Monospace"
        }
    }

    ComponentDetailsDialog {
        id: cdDialog
        title: qsTr("CD Details")

        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            text: nand.cdVersion
            font.family: "Monospace"
        }
    }

    ComponentDetailsDialog {
        id: ceDialog
        title: qsTr("CE Details")

        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            text: nand.ceVersion
            font.bold: true
            font.family: "Monospace"
        }
    }

    // The backend does not report XeLL details yet.
    ComponentDetailsDialog {
        id: xellDialog
        title: qsTr("XeLL Details")

        InfoField {
            Kirigami.FormData.label: qsTr("Version:")
            placeholderText: qsTr("Unknown")
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("Variant:")
            placeholderText: qsTr("Unknown")
        }
    }

    ComponentDetailsDialog {
        id: patch0Dialog
        title: qsTr("Patchslot 0 Details")
        implicitWidth: Kirigami.Units.gridUnit * 26

        InfoField {
            Kirigami.FormData.label: qsTr("CF Version:")
            text: nand.cf0Version
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("CG Version:")
            text: nand.cg0Version
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("CF LDV:")
            text: nand.cf0Ldv
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("CF PD:")
            text: nand.cf0Pairing
            font.family: "Monospace"
        }
    }

    ComponentDetailsDialog {
        id: patch1Dialog
        title: qsTr("Patchslot 1 Details")
        implicitWidth: Kirigami.Units.gridUnit * 26

        InfoField {
            Kirigami.FormData.label: qsTr("CF Version:")
            text: nand.cf1Version
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("CG Version:")
            text: nand.cg1Version
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("CF LDV:")
            text: nand.cf1Ldv
            font.family: "Monospace"
        }
        InfoField {
            Kirigami.FormData.label: qsTr("CF PD:")
            text: nand.cf1Pairing
            font.family: "Monospace"
        }
    }

    QQC2.Label {
        anchors.centerIn: parent
        visible: cardsView.count === 0
        text: root.enabled ? qsTr("No components discovered in this NAND image.") : qsTr("No components discovered. Load a NAND image to view components.")
        font.bold: true
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
        color: Kirigami.Theme.disabledTextColor
    }

    Kirigami.CardsListView {
        id: cardsView
        anchors.fill: parent
        model: nand.components

        delegate: DelegateChooser {
            role: "cardType"

            DelegateChoice {
                roleValue: "smc"
                ComponentCard {
                    heading: qsTr("SMC Firmware")
                    summary: root.versionSummary(modelData.versionStr, nand.smcVersion)
                    onDetailsRequested: smcDialog.open()
                }
            }

            DelegateChoice {
                roleValue: "cb"
                ComponentCard {
                    heading: qsTr("CB (2BL)")
                    summary: root.versionSummary(modelData.versionStr, nand.cbVersion)
                    onDetailsRequested: cbDialog.open()
                }
            }

            DelegateChoice {
                roleValue: "cb_a"
                ComponentCard {
                    heading: qsTr("CB_A (2BL)")
                    summary: root.versionSummary(modelData.versionStr, nand.cbAVersion)
                    onDetailsRequested: cbADialog.open()
                }
            }

            DelegateChoice {
                roleValue: "cb_b"
                ComponentCard {
                    heading: qsTr("CB_B (2BL)")
                    summary: root.versionSummary(modelData.versionStr, nand.cbBVersion)
                    onDetailsRequested: cbBDialog.open()
                }
            }

            DelegateChoice {
                roleValue: "cd"
                ComponentCard {
                    heading: qsTr("CD (4BL)")
                    summary: root.versionSummary(modelData.versionStr, nand.cdVersion)
                    onDetailsRequested: cdDialog.open()
                }
            }

            DelegateChoice {
                roleValue: "ce"
                ComponentCard {
                    heading: qsTr("CE (5BL / Kernel)")
                    summary: root.versionSummary(modelData.versionStr, nand.ceVersion)
                    onDetailsRequested: ceDialog.open()
                }
            }

            DelegateChoice {
                roleValue: "xell"
                ComponentCard {
                    heading: qsTr("XeLL")
                    summary: root.versionSummary(modelData.versionStr, "")
                    onDetailsRequested: xellDialog.open()
                }
            }

            DelegateChoice {
                roleValue: "patch0"
                ComponentCard {
                    heading: qsTr("Patchslot 0")
                    summary: root.versionSummary(modelData.versionStr, nand.cf0Version)
                    onDetailsRequested: patch0Dialog.open()
                }
            }

            DelegateChoice {
                roleValue: "patch1"
                ComponentCard {
                    heading: qsTr("Patchslot 1")
                    summary: root.versionSummary(modelData.versionStr, nand.cf1Version)
                    onDetailsRequested: patch1Dialog.open()
                }
            }

            DelegateChoice {
                roleValue: "keyvault"
                ComponentCard {
                    heading: qsTr("Keyvault")
                    summary: qsTr("Serial: %1").arg(modelData.versionStr ? modelData.versionStr : (nand.serialNumber !== "" ? nand.serialNumber : qsTr("Encrypted")))
                    onDetailsRequested: root.keyvaultRequested()
                }
            }
        }
    }
}
