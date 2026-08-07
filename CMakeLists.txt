cmake_minimum_required(VERSION 3.28)

project(genexis LANGUAGES CXX)

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
# Force static builds for gxbuild3/stfs/UpdClient: KDE/ECM sets BUILD_SHARED_LIBS=ON
# globally. Neither gxbuild3, stfs, nor UpdClient have symbol visibility annotations,
# so building them as shared libraries leaves all their symbols hidden and unresolvable.
# spdlog's local-exec TLS model is also incompatible with -shared.
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(extern/gxbuild3)
add_subdirectory(extern/UpdClient)
set(BUILD_SHARED_LIBS ON)

set(GENEXIS_SOURCES
    src/Main.cpp
    src/StartupManager.cpp
    src/Updater.cpp
    src/Utils.cpp
    src/api/Flasher.cpp
    src/api/Format.cpp
    src/api/Builder.cpp
    src/api/Metadata.cpp
    src/ini/IniParser.cpp
    src/vendor/USB.cpp
    src/plugins/PluginRegistry.cpp
    src/plugins/PluginManifest.cpp
    src/plugins/IPluginEndpoint.cpp
    src/plugins/endpoints/BuiltinEndpoint.cpp
    src/plugins/endpoints/DynamicLibraryEndpoint.cpp
    src/plugins/endpoints/ProcessEndpoint.cpp
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
    URI "org.gxgx.genexis"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        QML/Main.qml
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
)

add_dependencies(genexis gxbuild3_lib updclient_lib)

target_include_directories(genexis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/gxbuild3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/UpdClient/include
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(genexis PRIVATE
    gxbuild3_lib
    updclient_lib
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

include(GNUInstallDirs)
install(TARGETS genexis DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES org.gxgx.genexis.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES org.gxgx.genexis.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES org.gxgx.genexis.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)