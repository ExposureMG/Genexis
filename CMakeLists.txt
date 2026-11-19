cmake_minimum_required(VERSION 3.29)

project(genexis LANGUAGES C CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ENV{MAKEFLAGS} "")

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)

kde_enable_exceptions()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Genexis can only be compiled for 64-bit systems.")
endif()

if(NOT (WIN32 OR ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR APPLE))
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
endif()

ecm_find_qmlmodule(org.kde.kirigami REQUIRED)

find_package(Qt6 6.7 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickControls2
    Widgets
)

find_package(KF6 REQUIRED COMPONENTS
    Kirigami
    I18n
    CoreAddons
    QQC2DesktopStyle
    IconThemes
)

if(MSVC)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    add_compile_options(/permissive- /Zc:preprocessor)
else()
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Dasm=__asm__>)
    add_compile_options(-Wno-error=zero-as-null-pointer-constant -Wno-zero-as-null-pointer-constant -Wno-error=date-time -Wno-date-time)
endif()

block(SCOPE_FOR VARIABLES)
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(extern/gxbuild3)
endblock()

set(GENEXIS_CORE_SOURCES
    src/utils/Settings.cpp
    src/utils/Network.cpp
    src/utils/Files.cpp
    src/utils/files/XebuildIni.cpp
    src/adapters/Gxbuild3.cpp
    src/pages/Builder.cpp
    src/pages/Info.cpp
    include/utils/Settings.hpp
    include/utils/Network.hpp
    include/utils/Files.hpp
    include/utils/files/XebuildIni.hpp
    include/adapters/Gxbuild3.hpp
    include/pages/Builder.hpp
    include/pages/Info.hpp
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_library(genexis_core STATIC)

qt_add_qml_module(genexis_core
    URI "org.gxoss.genexis"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    SOURCES ${GENEXIS_CORE_SOURCES}
    QML_FILES
        gui/Main.qml
        gui/components/ConsoleDialog.qml
        gui/components/KeyvaultSheet.qml
        gui/components/NandBuilder.qml
        gui/components/NandBuilderAdvanced.qml
        gui/components/NandBuilderDonor.qml
        gui/components/NandBuilderSimple.qml
        gui/components/StatusBar.qml
        gui/components/nand/Components.qml
        gui/components/nand/Keyvault.qml
        gui/components/nand/Overview.qml
        gui/components/nand/parts/Bootloaders.qml
        gui/components/nand/parts/Kernel.qml
        gui/components/nand/parts/Smc.qml
        gui/components/nand/parts/Updates.qml
        gui/pages/BadUpdate.qml
        gui/pages/Flasher.qml
        gui/pages/Home.qml
        gui/pages/NandBuilder.qml
        gui/pages/NandInfo.qml
        gui/pages/Settings.qml
    RESOURCES
        gui/assets/icons/loading-gear.svg
        gui/assets/icons/loading-gear-alt.svg
        gui/assets/icons/settings-black.svg
        gui/assets/icons/settings-trans.svg
        gui/assets/vendor/demon.png
        gui/assets/vendor/jrp.png
        gui/assets/vendor/mtx.png
        gui/assets/vendor/nandx.png
        gui/assets/vendor/noflasher.png
        gui/assets/vendor/pico-djtag.png
        gui/assets/vendor/picoflasher.png
        gui/assets/vendor/xflasher.png
)

target_include_directories(genexis_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(genexis_core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
    PRIVATE
        gxbuild3_lib
        KF6::CoreAddons
        KF6::I18n
        KF6::Kirigami
)

qt_add_executable(genexis src/Main.cpp)

target_link_libraries(genexis PRIVATE
    genexis_core
    genexis_coreplugin
    Qt6::Widgets
    KF6::IconThemes
    KF6::Kirigami
)

if(ANDROID)
    target_link_libraries(genexis PRIVATE log)
endif()

include(GNUInstallDirs)
install(TARGETS genexis
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES gui/metadata/org.gxoss.genexis.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES gui/metadata/org.gxoss.genexis.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES gui/assets/app/org.gxoss.genexis.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
