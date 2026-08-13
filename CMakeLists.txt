cmake_minimum_required(VERSION 3.28)

project(genexis LANGUAGES C CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Clear MAKEFLAGS environment variable to prevent cargo build scripts from inheriting parent make flags
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


# x86-64 Only
if(NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|x64)$" OR CMAKE_VS_PLATFORM_NAME STREQUAL "x64"))
    message(FATAL_ERROR "Genexis can only be compiled on x86-64 architecture.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Genexis can only be compiled for 64-bit (x86-64) target.")
endif()

# OS and Compiler Guards
if(WIN32)
    # If windows: MSVC only
    if(NOT MSVC)
        message(FATAL_ERROR "Windows builds require MSVC compiler.")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # If Linux: G++ or Clang++ only
    if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        message(FATAL_ERROR "Linux builds require GCC (g++) or Clang (clang++) compiler.")
    endif()
elseif(APPLE)
    # If MacOS: Clang++ only
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "MacOS builds require Clang++ compiler.")
    endif()
else()
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
)

find_package(KF6 REQUIRED COMPONENTS
    Kirigami
    I18n
    CoreAddons
    QQC2DesktopStyle
    IconThemes
)

add_compile_options($<$<COMPILE_LANGUAGE:C>:-Dasm=__asm__>)
add_compile_options(-Wno-error=zero-as-null-pointer-constant -Wno-zero-as-null-pointer-constant -Wno-error=date-time -Wno-date-time)

# Subdirectories to compile with Genexis
# Force static builds for gxbuild3/stfs/UpdClient/FTDI2SPI/xsvftool: KDE/ECM sets BUILD_SHARED_LIBS=ON
# globally. Neither gxbuild3, stfs, nor UpdClient have symbol visibility annotations,
# so building them as shared libraries leaves all their symbols hidden and unresolvable.
# spdlog's local-exec TLS model is also incompatible with -shared.
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(extern/gxbuild3)
add_subdirectory(extern/UpdClient)
add_subdirectory(extern/xsvftool)

set(FTDI2SPI_SOURCES
    extern/FTDI2SPI/src/Main.cpp
    extern/FTDI2SPI/src/Flasher.cpp
    extern/FTDI2SPI/src/XNAND.cpp
    extern/FTDI2SPI/src/XSPI.cpp
    extern/FTDI2SPI/src/XeMMC.cpp
    extern/FTDI2SPI/src/sfc.cpp
    extern/FTDI2SPI/src/stdafx.cpp
    extern/FTDI2SPI/src/wrapper_spi.cpp
)
add_library(FTDI2SPI STATIC ${FTDI2SPI_SOURCES})
target_compile_definitions(FTDI2SPI PRIVATE -Dmain=ftdi2spi_dummy_main _CRT_SECURE_NO_WARNINGS)
target_include_directories(FTDI2SPI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/FTDI2SPI/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xsvftool/extern/libftdi/src
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xsvftool/extern/libusb/libusb
        ${CMAKE_CURRENT_BINARY_DIR}/extern/xsvftool/extern/libftdi/src
)
target_link_libraries(FTDI2SPI PRIVATE ftdi1-static)

set(BUILD_SHARED_LIBS ON)

# Build NandProMax static library with Cargo
set(NANDPROMAX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/NandProMax)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
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
    src/vendor/USB.cpp
    src/vendor/USBDiscovery.cpp
    src/backend/BackendManager.cpp
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
    include/vendor/USBDiscovery.hpp
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

if(WIN32)
    list(APPEND GENEXIS_SOURCES
        src/vendor/platforms/Windows.cpp
    )
else()
    list(APPEND GENEXIS_SOURCES
        src/vendor/platforms/UNIX.cpp
    )
endif()

qt_add_executable(genexis ${GENEXIS_SOURCES})

qt_policy(SET QTP0004 NEW)

qt_add_qml_module(genexis
    URI "org.gxoss.genexis"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        QML/Main.qml
        QML/components/ConsoleDialog.qml
        QML/components/NandBuilder.qml
        QML/components/NandBuilderAdvanced.qml
        QML/components/NandBuilderDonor.qml
        QML/components/NandBuilderSimple.qml
        QML/components/StatusBar.qml
        QML/pages/BadUpdate.qml
        QML/pages/Flasher.qml
        QML/pages/Home.qml
        QML/pages/Nand.qml
        QML/pages/NandBuilder.qml
        QML/pages/Settings.qml
        QML/pages/nand/Bootloaders.qml
        QML/pages/nand/Components.qml
        QML/pages/nand/Kernel.qml
        QML/pages/nand/Keyvault.qml
        QML/pages/nand/Overview.qml
        QML/pages/nand/Smc.qml
        QML/pages/nand/Updates.qml
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
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xsvftool/include
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
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
)

if(WIN32)
    target_link_libraries(genexis PRIVATE setupapi winusb advapi32 user32 ws2_32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    pkg_check_modules(LIBUDEV REQUIRED libudev)
    target_include_directories(genexis PRIVATE ${LIBUSB_INCLUDE_DIRS} ${LIBUDEV_INCLUDE_DIRS})
    target_link_libraries(genexis PRIVATE ${LIBUSB_LIBRARIES} ${LIBUDEV_LIBRARIES} pthread dl m)
endif()

if(NOT WIN32)
    target_link_options(genexis PRIVATE -Wl,--allow-multiple-definition)
endif()

include(GNUInstallDirs)
install(TARGETS genexis DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES org.gxoss.genexis.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES org.gxoss.genexis.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES org.gxoss.genexis.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)