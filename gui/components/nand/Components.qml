import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.gxoss.genexis

Kirigami.ScrollablePage {
    id: root

    title: qsTr("Components")

    signal keyvaultRequested

    enabled: NandSession.isNandLoaded
    opacity: enabled ? 1.0 : 0.45

    property string xellVersion: "0.99"
    property string xellType: "XeLL Reloaded"
    property string activeDialogType: ""

    function cardInfo(cardType, versionStr) {
        switch (cardType) {
        case "smc":
            return {
                title: qsTr("SMC Firmware"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.smcVersion)
            };
        case "cb":
            return {
                title: qsTr("CB (2BL)"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.cbVersion)
            };
        case "cb_a":
            return {
                title: qsTr("CB_A (2BL)"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.cbAVersion)
            };
        case "cb_b":
            return {
                title: qsTr("CB_B (2BL)"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.cbBVersion)
            };
        case "cd":
            return {
                title: qsTr("CD (4BL)"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.cdVersion)
            };
        case "ce":
            return {
                title: qsTr("CE (5BL / Kernel)"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.ceVersion)
            };
        case "xell":
            return {
                title: qsTr("XeLL"),
                summary: qsTr("Version: %1").arg(versionStr || root.xellVersion)
            };
        case "patch0":
            return {
                title: qsTr("Patchslot 0"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.cf0Version)
            };
        case "patch1":
            return {
                title: qsTr("Patchslot 1"),
                summary: qsTr("Version: %1").arg(versionStr || InfoController.cf1Version)
            };
        case "keyvault":
            return {
                title: qsTr("Keyvault"),
                summary: qsTr("Serial: %1").arg(versionStr || (InfoController.serialNumber !== "" ? InfoController.serialNumber : qsTr("Encrypted")))
            };
        default:
            return {
                title: "",
                summary: ""
            };
        }
    }

    function detailSpec(cardType) {
        switch (cardType) {
        case "smc":
            return {
                title: qsTr("SMC Firmware Details"),
                widthUnits: 24,
                fields: [
                    { label: qsTr("Target:"), value: NandSession.consoleTarget },
                    { label: qsTr("Version:"), value: InfoController.smcVersion, monospace: true },
                    { label: qsTr("Type:"), value: InfoController.smcType }
                ]
            };
        case "cb":
            return {
                title: qsTr("CB Details"),
                widthUnits: 26,
                fields: [
                    { label: qsTr("Version:"), value: InfoController.cbVersion, monospace: true },
                    { label: qsTr("Size:"), value: InfoController.cbSize !== "" ? InfoController.cbSize + " bytes" : "", monospace: true },
                    { label: qsTr("Magic:"), value: InfoController.cbMagic, monospace: true },
                    { label: qsTr("LDV:"), value: InfoController.cbLdv, monospace: true },
                    { label: qsTr("PD:"), value: InfoController.cbPairing, monospace: true }
                ]
            };
        case "cb_a":
            return {
                title: qsTr("CB_A Details"),
                widthUnits: 26,
                fields: [
                    { label: qsTr("Version:"), value: InfoController.cbAVersion, monospace: true },
                    { label: qsTr("LDV:"), value: InfoController.cbALdv, monospace: true },
                    { label: qsTr("PD:"), value: InfoController.cbAPairing, monospace: true }
                ]
            };
        case "cb_b":
            return {
                title: qsTr("CB_B Details"),
                widthUnits: 24,
                fields: [
                    { label: qsTr("Version:"), value: InfoController.cbBVersion, monospace: true }
                ]
            };
        case "cd":
            return {
                title: qsTr("CD Details"),
                widthUnits: 24,
                fields: [
                    { label: qsTr("Version:"), value: InfoController.cdVersion, monospace: true }
                ]
            };
        case "ce":
            return {
                title: qsTr("CE Details"),
                widthUnits: 24,
                fields: [
                    { label: qsTr("Version:"), value: InfoController.ceVersion, monospace: true, bold: true }
                ]
            };
        case "xell":
            return {
                title: qsTr("XeLL Details"),
                widthUnits: 24,
                fields: [
                    { label: qsTr("Version:"), value: root.xellVersion, monospace: true, placeholder: "" },
                    { label: qsTr("Variant:"), value: root.xellType, placeholder: "" }
                ]
            };
        case "patch0":
            return {
                title: qsTr("Patchslot 0 Details"),
                widthUnits: 26,
                fields: [
                    { label: qsTr("CF Version:"), value: InfoController.cf0Version, monospace: true },
                    { label: qsTr("CG Version:"), value: InfoController.cg0Version, monospace: true },
                    { label: qsTr("CF LDV:"), value: InfoController.cf0Ldv, monospace: true },
                    { label: qsTr("CF PD:"), value: InfoController.cf0Pairing, monospace: true }
                ]
            };
        case "patch1":
            return {
                title: qsTr("Patchslot 1 Details"),
                widthUnits: 26,
                fields: [
                    { label: qsTr("CF Version:"), value: InfoController.cf1Version, monospace: true },
                    { label: qsTr("CG Version:"), value: InfoController.cg1Version, monospace: true },
                    { label: qsTr("CF LDV:"), value: InfoController.cf1Ldv, monospace: true },
                    { label: qsTr("CF PD:"), value: InfoController.cf1Pairing, monospace: true }
                ]
            };
        default:
            return {
                title: "",
                widthUnits: 24,
                fields: []
            };
        }
    }

    ComponentDetailDialog {
        id: detailDialog
        readonly property var spec: root.detailSpec(root.activeDialogType)
        title: spec.title
        preferredWidth: Kirigami.Units.gridUnit * spec.widthUnits
        fields: spec.fields
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        visible: cardsView.count === 0
        icon.name: "package-x-generic"
        text: root.enabled ? qsTr("No Components Discovered") : qsTr("No NAND Image Loaded")
        explanation: root.enabled ? qsTr("No components were found in this NAND image.") : qsTr("Load a NAND image to inspect its components.")
    }

    Kirigami.CardsListView {
        id: cardsView
        anchors.fill: parent
        model: InfoController.components

        delegate: ComponentCard {
            required property var modelData
            readonly property var info: root.cardInfo(modelData.cardType, modelData.versionStr)

            title: info.title
            summaryText: info.summary
            onDetailsClicked: {
                if (modelData.cardType === "keyvault") {
                    root.keyvaultRequested();
                } else {
                    root.activeDialogType = modelData.cardType;
                    detailDialog.open();
                }
            }
        }
    }
}
