qt_add_qml_module(genexis
    URI "org.gxgx.genexis"
    VERSION 1.0
    NO_RESOURCE_TARGET_ALIAS
    QML_FILES
        Main.qml
        components/NandBuilder.qml
        components/NandBuilderAdvanced.qml
        components/NandBuilderSimple.qml
        components/StatusBar.qml
        pages/BadUpdate.qml
        pages/Flasher.qml
        pages/Home.qml
        pages/Nand.qml
        pages/NandBuilder.qml
        pages/Settings.qml
        pages/nand/Bootloaders.qml
        pages/nand/Components.qml
        pages/nand/Kernel.qml
        pages/nand/Keyvault.qml
        pages/nand/Overview.qml
        pages/nand/Smc.qml
        pages/nand/Updates.qml
)
