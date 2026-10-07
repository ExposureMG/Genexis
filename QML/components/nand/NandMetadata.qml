import QtQuick

// The only place QML reads nandController.metadata (grouped as header, smc,
// bootloaders, keyvault and components; see NandInfoToVariantMap). Views bind
// to the flat properties of an instance of this object. Load-state flags
// (isLoading, isNandLoaded, ...) are read from nandController directly.
QtObject {
    readonly property var metadata: nandController.metadata

    readonly property var components: metadata.components

    readonly property string consoleTarget: metadata.header.consoleTarget
    readonly property string imageSize: metadata.header.imageSize
    readonly property string blockType: metadata.header.blockType
    readonly property string kernelVerOrType: metadata.header.kernelVerOrType

    readonly property string smcVersion: metadata.smc.version
    readonly property string smcType: metadata.smc.type
    readonly property string smcSize: metadata.smc.size
    readonly property string smcConfigOffset: metadata.smc.configOffset

    readonly property string cbVersion: metadata.bootloaders.cbVersion
    readonly property string cbSize: metadata.bootloaders.cbSize
    readonly property string cbMagic: metadata.bootloaders.cbMagic
    readonly property string cbLdv: metadata.bootloaders.cbLdv
    readonly property string cbPairing: metadata.bootloaders.cbPairing
    readonly property string cbAVersion: metadata.bootloaders.cbAVersion
    readonly property string cbALdv: metadata.bootloaders.cbALdv
    readonly property string cbAPairing: metadata.bootloaders.cbAPairing
    readonly property string cbBVersion: metadata.bootloaders.cbBVersion

    readonly property string scVersion: metadata.bootloaders.scVersion
    readonly property string ccVersion: metadata.bootloaders.ccVersion
    readonly property string cdVersion: metadata.bootloaders.cdVersion
    readonly property string ceVersion: metadata.bootloaders.ceVersion

    readonly property string cf0Version: metadata.bootloaders.cf0Version
    readonly property string cg0Version: metadata.bootloaders.cg0Version
    readonly property string cf0Ldv: metadata.bootloaders.cf0Ldv
    readonly property string cf0Pairing: metadata.bootloaders.cf0Pairing
    readonly property string cf1Version: metadata.bootloaders.cf1Version
    readonly property string cg1Version: metadata.bootloaders.cg1Version
    readonly property string cf1Ldv: metadata.bootloaders.cf1Ldv
    readonly property string cf1Pairing: metadata.bootloaders.cf1Pairing

    readonly property string serialNumber: metadata.keyvault.serialNumber
    readonly property string consoleId: metadata.keyvault.consoleId
    readonly property string dvdKey: metadata.keyvault.dvdKey
    readonly property string gameRegion: metadata.keyvault.gameRegion
    readonly property string consoleType: metadata.keyvault.consoleType
}
