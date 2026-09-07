cmake_minimum_required(VERSION 3.28)

include(cmake/GenexisCompiler.cmake)
genexis_select_compilers()

project(genexis LANGUAGES C CXX)
genexis_validate_compilers()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENV{MAKEFLAGS} "")

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMQmlModule)

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

set(UPDCLIENT_BUILD_CLI OFF CACHE BOOL "" FORCE)

ecm_find_qmlmodule(org.kde.kirigami REQUIRED)

find_package(ZLIB REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    Test
    Gui
    QuickControls2
    Widgets
    Network
    SerialPort
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

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(extern/UpdClient)

if(MSVC)
    include(cmake/GenexisGxBuild3MSVC.cmake)
else()
    add_subdirectory(extern/gxbuild3)
endif()


add_subdirectory(extern/xsvftool)
add_subdirectory(extern/FTDI2SPI)

set(BUILD_SHARED_LIBS ON)

set(NANDPROMAX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/NandProMax)
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CARGO_BUILD_FLAG "--release")
    set(NANDPROMAX_BUILD_DIR ${NANDPROMAX_DIR}/target/release)
else()
    set(CARGO_BUILD_FLAG "")
    set(NANDPROMAX_BUILD_DIR ${NANDPROMAX_DIR}/target/debug)
endif()

if(WIN32)
    set(NANDPROMAX_LIB ${NANDPROMAX_BUILD_DIR}/nandpromax.lib)
else()
    set(NANDPROMAX_LIB ${NANDPROMAX_BUILD_DIR}/libnandpromax.a)
endif()

add_custom_command(
    OUTPUT ${NANDPROMAX_LIB}
    COMMAND cargo build ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${NANDPROMAX_DIR}
    COMMENT "Building NandProMax static library via Cargo..."
)

add_custom_target(nandpromax_target DEPENDS ${NANDPROMAX_LIB})
add_library(nandpromax_lib STATIC IMPORTED GLOBAL)
add_dependencies(nandpromax_lib nandpromax_target)
set_target_properties(nandpromax_lib PROPERTIES IMPORTED_LOCATION ${NANDPROMAX_LIB})

set(GENEXIS_SOURCES
    src/Main.cpp
    src/StartupManager.cpp
    src/Updater.cpp
    src/Utils.cpp
    src/api/Format.cpp
    src/backend/BackendManager.cpp
    src/backend/FlasherDiscovery.cpp
    src/backend/adapters/NandProMaxAdapter.cpp
    src/backend/adapters/Ftdi2SpiAdapter.cpp
    src/backend/adapters/XsvfToolAdapter.cpp
    src/backend/adapters/UpdClientAdapter.cpp
    src/backend/adapters/GxBuild3Adapter.cpp
    src/pages/Home.cpp
    src/pages/Explorer.cpp
    src/pages/Editor.cpp
    src/pages/Nand.cpp
    src/pages/NandBuilderController.cpp
    src/pages/Flasher.cpp
    src/pages/BadUpdate.cpp
    src/pages/Network.cpp
    src/pages/Settings.cpp
    include/StartupManager.hpp
    include/backend/FlasherDiscovery.hpp
    include/backend/IFlashService.hpp
    include/backend/IJtagService.hpp
    include/backend/IBuilderService.hpp
    include/backend/INetworkService.hpp
    include/backend/FlasherDeviceRegistry.hpp
    include/backend/BackendManager.hpp
    include/backend/adapters/NandProMaxAdapter.hpp
    include/backend/adapters/Ftdi2SpiAdapter.hpp
    include/backend/adapters/XsvfToolAdapter.hpp
    include/backend/adapters/UpdClientAdapter.hpp
    include/backend/adapters/GxBuild3Adapter.hpp
    include/pages/Home.hpp
    include/pages/Explorer.hpp
    include/pages/Editor.hpp
    include/pages/Nand.hpp
    include/pages/NandBuilderController.hpp
    include/pages/Flasher.hpp
    include/pages/BadUpdate.hpp
    include/pages/Network.hpp
    include/pages/Settings.hpp
)

qt_add_executable(genexis ${GENEXIS_SOURCES})

qt_policy(SET QTP0004 NEW)

qt_add_qml_module(genexis
    URI "org.gxoss.genexis"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        QML/Main.qml
        QML/components/ConsoleDialog.qml
        QML/components/KeyvaultSheet.qml
        QML/components/NandBuilder.qml
        QML/components/NandBuilderAdvanced.qml
        QML/components/NandBuilderDonor.qml
        QML/components/NandBuilderSimple.qml
        QML/components/StatusBar.qml
        QML/components/nand/Components.qml
        QML/components/nand/Keyvault.qml
        QML/components/nand/Overview.qml
        QML/components/nand/parts/Bootloaders.qml
        QML/components/nand/parts/Kernel.qml
        QML/components/nand/parts/Smc.qml
        QML/components/nand/parts/Updates.qml
        QML/pages/BadUpdate.qml
        QML/pages/Flasher.qml
        QML/pages/Home.qml
        QML/pages/NandBuilder.qml
        QML/pages/NandInfo.qml
        QML/pages/Settings.qml
    RESOURCES
        assets/demon.png
        assets/jrp.png
        assets/mtx.png
        assets/nandx.png
        assets/loading-gear.svg
        assets/loading-gear-alt.svg
        assets/noflasher.png
        assets/picoflasher.png
        assets/xflasher.png
)

add_dependencies(genexis gxbuild3_lib updclient_lib FTDI2SPI xsvftool nandpromax_target)

target_include_directories(genexis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/gxbuild3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/UpdClient/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/NandProMax/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/FTDI2SPI/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xsvftool/xsvftool
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xsvftool/libxsvf
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(genexis PRIVATE
    nandpromax_lib
    gxbuild3_lib
    updclient_lib
    FTDI2SPI
    xsvftool
    stfs
    ZLIB::ZLIB
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
)

if(WIN32)
    target_link_libraries(genexis PRIVATE advapi32 user32 ws2_32 setupapi winusb)
elseif(ANDROID)
    target_link_libraries(genexis PRIVATE log)
elseif(APPLE)
    target_link_libraries(genexis PRIVATE
        "-framework CoreFoundation"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(genexis PRIVATE pthread dl m)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(genexis PRIVATE -Wl,--allow-multiple-definition)
endif()

include(GNUInstallDirs)
install(TARGETS genexis FTDI2SPI xsvftool
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES org.gxoss.genexis.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES org.gxoss.genexis.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES org.gxoss.genexis.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
