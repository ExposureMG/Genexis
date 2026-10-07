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

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMQmlModule)

# Must run before the bundled projects below, which change BUILD_TESTING.
include(CTest)

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

set(GENEXIS_QT_COMPONENTS
    Core
    Quick
    Gui
    QuickControls2
    Widgets
    Network
    SerialPort
)
if(BUILD_TESTING)
    list(APPEND GENEXIS_QT_COMPONENTS Test)
endif()
find_package(Qt6 REQUIRED COMPONENTS ${GENEXIS_QT_COMPONENTS})

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

# The bundled libraries are built static (KDECMakeSettings defaults
# BUILD_SHARED_LIBS to ON, and they carry no symbol export annotations) and
# without their own test suites. Both are normal variables here, so the cache
# keeps the user's values; UpdClient force-sets BUILD_TESTING OFF in the cache,
# which is undone below.
set(GENEXIS_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)

# gxbuild3 adds its bundled spdlog unconditionally. Configure it first so
# UpdClient reuses that target instead of fetching a second copy.
if(NOT MSVC)
    add_subdirectory(extern/gxbuild3)
endif()

add_subdirectory(extern/UpdClient)

if(MSVC)
    include(cmake/GenexisGxBuild3MSVC.cmake)
endif()

# xsvftool's bundled libftdi would also build a shared libftdi1.so.2 into bin/,
# which is on genexis's link rpath. Built with hidden visibility it exports
# nothing, yet ld picks it for libFTDI2SPI.so's DT_NEEDED libftdi1.so.2 instead
# of the system library, and the genexis link fails with "hidden symbol
# `ftdi_...' is referenced by DSO". Only its static library is used.
set(SHAREDLIBS OFF)
add_subdirectory(extern/xsvftool)
unset(SHAREDLIBS)
add_subdirectory(extern/FTDI2SPI)

unset(BUILD_SHARED_LIBS)
unset(BUILD_TESTING)
set(BUILD_TESTING ${GENEXIS_BUILD_TESTING}
    CACHE BOOL "Build the testing tree." FORCE)

find_program(CARGO_EXECUTABLE cargo
    HINTS "$ENV{CARGO_HOME}/bin" "$ENV{HOME}/.cargo/bin"
)
if(NOT CARGO_EXECUTABLE)
    message(FATAL_ERROR
        "cargo was not found. NandProMax is a Rust crate: install a Rust "
        "toolchain (https://rustup.rs) and make sure cargo is on PATH, or "
        "pass -DCARGO_EXECUTABLE=/path/to/cargo.")
endif()

set(NANDPROMAX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/NandProMax)
set(NANDPROMAX_TARGET_DIR ${CMAKE_BINARY_DIR}/nandpromax)
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CARGO_BUILD_FLAGS --release)
    set(NANDPROMAX_BUILD_DIR ${NANDPROMAX_TARGET_DIR}/release)
else()
    set(CARGO_BUILD_FLAGS "")
    set(NANDPROMAX_BUILD_DIR ${NANDPROMAX_TARGET_DIR}/debug)
endif()

# Only a lockfile the submodule commits is authoritative; a local one written
# by an earlier cargo run may be stale after a submodule update.
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} ls-files --error-unmatch Cargo.lock
        WORKING_DIRECTORY ${NANDPROMAX_DIR}
        RESULT_VARIABLE NANDPROMAX_LOCK_STATUS
        OUTPUT_QUIET ERROR_QUIET
    )
    if(NANDPROMAX_LOCK_STATUS EQUAL 0)
        list(APPEND CARGO_BUILD_FLAGS --locked)
    endif()
endif()

if(WIN32)
    set(NANDPROMAX_LIB ${NANDPROMAX_BUILD_DIR}/nandpromax.lib)
else()
    set(NANDPROMAX_LIB ${NANDPROMAX_BUILD_DIR}/libnandpromax.a)
endif()

# Always invoke cargo (a no-op when nothing changed) so updating the submodule
# rebuilds the library instead of linking a stale one. Build output goes to the
# CMake binary dir so the submodule checkout stays clean.
add_custom_target(nandpromax_target ALL
    COMMAND ${CARGO_EXECUTABLE} build ${CARGO_BUILD_FLAGS}
        --target-dir ${NANDPROMAX_TARGET_DIR}
    WORKING_DIRECTORY ${NANDPROMAX_DIR}
    BYPRODUCTS ${NANDPROMAX_LIB}
    COMMENT "Building NandProMax static library via Cargo..."
    VERBATIM
)
add_library(nandpromax_lib STATIC IMPORTED GLOBAL)
add_dependencies(nandpromax_lib nandpromax_target)
set_target_properties(nandpromax_lib PROPERTIES IMPORTED_LOCATION ${NANDPROMAX_LIB})

set(GENEXIS_SOURCES
    src/Main.cpp
    src/StartupManager.cpp
    src/backend/BackendManager.cpp
    src/backend/FlasherDiscovery.cpp
    src/backend/adapters/NandProMaxAdapter.cpp
    src/backend/adapters/Ftdi2SpiAdapter.cpp
    src/backend/adapters/XsvfToolAdapter.cpp
    src/backend/adapters/UpdClientAdapter.cpp
    src/backend/adapters/GxBuild3Adapter.cpp
    src/pages/Nand.cpp
    src/pages/NandInfoMapper.cpp
    src/pages/NandBuilderController.cpp
    src/pages/Flasher.cpp
    src/pages/Settings.cpp
    src/pages/FlasherOperations.hpp
    src/pages/NandBuildConfigMapper.hpp
    include/Async.hpp
    include/FilePaths.hpp
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
    include/pages/Nand.hpp
    include/pages/NandInfoMapper.hpp
    include/pages/NandBuilderController.hpp
    include/pages/Flasher.hpp
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
        QML/components/BuildButton.qml
        QML/components/ConsoleDialog.qml
        QML/components/KeyvaultSheet.qml
        QML/components/NandBuilderAdvanced.qml
        QML/components/NandBuilderDonor.qml
        QML/components/NandBuilderSimple.qml
        QML/components/NandBuilderTabs.qml
        QML/components/PatchesDialog.qml
        QML/components/PatchesRow.qml
        QML/components/SelectorCombo.qml
        QML/components/StatusBar.qml
        QML/components/nand/ComponentCard.qml
        QML/components/nand/ComponentDetailsDialog.qml
        QML/components/nand/Components.qml
        QML/components/nand/InfoField.qml
        QML/components/nand/Keyvault.qml
        QML/components/nand/NandMetadata.qml
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
    KF6::IconThemes
)

if(BUILD_TESTING)
    # Usage requirements every test target shares.
    add_library(genexis_test_support INTERFACE)
    target_include_directories(genexis_test_support INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(genexis_test_support INTERFACE
        Qt6::Test
        Qt6::Core
    )

    # genexis_add_test(<name> <sources>... [LIBRARIES <targets>...])
    # builds genexis_<name>_tests and registers it with CTest as <name>.
    function(genexis_add_test name)
        cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "LIBRARIES")
        set(target genexis_${name}_tests)
        qt_add_executable(${target} ${arg_UNPARSED_ARGUMENTS})
        target_link_libraries(${target} PRIVATE
            genexis_test_support
            ${arg_LIBRARIES}
        )
        add_test(NAME ${name} COMMAND ${target})
    endfunction()

    set(GENEXIS_TEST_MAPPER_SOURCES
        src/pages/NandInfoMapper.cpp
        include/pages/NandInfoMapper.hpp
    )
    set(GENEXIS_TEST_NAND_SOURCES
        ${GENEXIS_TEST_MAPPER_SOURCES}
        src/pages/Nand.cpp
        include/pages/Nand.hpp
    )

    genexis_add_test(nand_info_mapper
        tests/NandInfoMapperTests.cpp tests/NandSnapshotFixture.hpp
        ${GENEXIS_TEST_MAPPER_SOURCES} LIBRARIES gxbuild3_lib)
    genexis_add_test(gxbuild3_adapter
        tests/GxBuild3AdapterTests.cpp
        src/backend/adapters/GxBuild3Adapter.cpp
        include/backend/adapters/GxBuild3Adapter.hpp
        LIBRARIES gxbuild3_lib)
    genexis_add_test(flasher_device_registry
        tests/FlasherDeviceRegistryTests.cpp
        include/backend/FlasherDeviceRegistry.hpp)
    genexis_add_test(nand_load
        tests/NandLoadTests.cpp
        ${GENEXIS_TEST_NAND_SOURCES} LIBRARIES gxbuild3_lib)
    genexis_add_test(nand_model
        tests/NandModelTests.cpp tests/NandSnapshotFixture.hpp
        ${GENEXIS_TEST_NAND_SOURCES} LIBRARIES gxbuild3_lib)

    # Loads the NAND QML views from the source tree against a real Nand.
    genexis_add_test(nand_qml
        tests/NandQmlTests.cpp tests/NandSnapshotFixture.hpp
        ${GENEXIS_TEST_NAND_SOURCES} LIBRARIES Qt6::Quick gxbuild3_lib)
    target_compile_definitions(genexis_nand_qml_tests PRIVATE
        GENEXIS_QML_DIR="${CMAKE_CURRENT_SOURCE_DIR}/QML"
    )
    # Home.qml loads this asset by its app qrc path.
    qt_add_resources(genexis_nand_qml_tests genexis_nand_qml_test_assets
        PREFIX /qt/qml/org/gxoss/genexis
        FILES assets/loading-gear.svg
    )

    genexis_add_test(async tests/AsyncTests.cpp include/Async.hpp)
    genexis_add_test(flasher_operations
        tests/FlasherOperationsTests.cpp
        include/FilePaths.hpp
        src/pages/FlasherOperations.hpp)
endif()

if(WIN32)
    target_link_libraries(genexis PRIVATE advapi32 user32 ws2_32 setupapi winusb)
elseif(ANDROID)
    target_link_libraries(genexis PRIVATE log)
elseif(APPLE)
    target_link_libraries(genexis PRIVATE
        "-framework CoreFoundation"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # libnandpromax (Rust staticlib) needs libudev (serialport) and libusb (rusb)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NANDPROMAX_SYSLIBS REQUIRED IMPORTED_TARGET libudev libusb-1.0)
    target_link_libraries(genexis PRIVATE PkgConfig::NANDPROMAX_SYSLIBS pthread dl m rt util)
endif()

include(GNUInstallDirs)
install(TARGETS genexis FTDI2SPI xsvftool
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES org.gxoss.genexis.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES org.gxoss.genexis.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES org.gxoss.genexis.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
