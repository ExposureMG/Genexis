set(GXBUILD3_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/extern/gxbuild3)

if(NOT TARGET spdlog::spdlog)
    add_library(spdlog INTERFACE)
    add_library(spdlog::spdlog ALIAS spdlog)
    target_include_directories(spdlog INTERFACE ${GXBUILD3_ROOT}/extern/spdlog/include)
endif()

if(NOT TARGET argparse::argparse)
    add_library(argparse INTERFACE)
    add_library(argparse::argparse ALIAS argparse)
    target_include_directories(argparse INTERFACE ${GXBUILD3_ROOT}/extern/argparse/include)
endif()

set(GXCRYPT_SOURCES
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_aes.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_bn_key.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_bn_mod.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_bn_pkcs1.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_bn_rsa.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_bn_sig.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_bn.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_des.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_ecc.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_md5.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_mem.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_parve.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_rc4.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_rotsum.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_sha.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_sha2.c
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/excrypt_uid_ecc.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/exkeys.cpp
    ${GXBUILD3_ROOT}/extern/GxCrypt/src/rijndael.c
)

add_library(GxCrypt STATIC ${GXCRYPT_SOURCES})
target_compile_features(GxCrypt PRIVATE cxx_std_23)
target_include_directories(GxCrypt
    PUBLIC
        ${GXBUILD3_ROOT}/extern/GxCrypt/include
    PRIVATE
        ${GXBUILD3_ROOT}/extern/GxCrypt/src
)
target_compile_definitions(GxCrypt PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(GxCrypt PRIVATE bcrypt)

set(STFS_SOURCES
    ${GXBUILD3_ROOT}/extern/stfs/src/FileTableParser.cpp
    ${GXBUILD3_ROOT}/extern/stfs/src/HeaderParser.cpp
    ${GXBUILD3_ROOT}/extern/stfs/src/MetadataParser.cpp
    ${GXBUILD3_ROOT}/extern/stfs/src/BlockParser.cpp
    ${GXBUILD3_ROOT}/extern/stfs/src/FileExtractor.cpp
    ${GXBUILD3_ROOT}/extern/stfs/src/HashVerifier.cpp
    ${GXBUILD3_ROOT}/extern/stfs/src/Package.cpp
)

add_library(stfs STATIC ${STFS_SOURCES})
target_compile_features(stfs PRIVATE cxx_std_23)
target_include_directories(stfs
    PUBLIC
        ${GXBUILD3_ROOT}/extern/stfs/include
    PRIVATE
        ${GXBUILD3_ROOT}/extern/stfs/src
)
target_compile_definitions(stfs PRIVATE _CRT_SECURE_NO_WARNINGS)

set(GXBUILD3_LIB_SOURCES
    ${GXBUILD3_ROOT}/src/Args.cpp
    ${GXBUILD3_ROOT}/src/BuildRunner.cpp
    ${GXBUILD3_ROOT}/src/Library.cpp
    ${GXBUILD3_ROOT}/src/ini/IniParser.cpp
    ${GXBUILD3_ROOT}/src/nand/FlashDriver.cpp
    ${GXBUILD3_ROOT}/src/nand/FlashImage.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/2bl.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/3bl.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/4bl.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/5bl.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/6bl.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/7bl.cpp
    ${GXBUILD3_ROOT}/src/nand/bootloaders/BootloaderPacker.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/CoronaConfig.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/FlashFileSystem.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/Keyvault.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/MobileData.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/Patchset.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/SMC.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/XConfig.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/Xboxupd.cpp
    ${GXBUILD3_ROOT}/src/nand/objects/XeLL.cpp
    ${GXBUILD3_ROOT}/src/patchers/Patcher.cpp
    ${GXBUILD3_ROOT}/src/patchers/Signature.cpp
    ${GXBUILD3_ROOT}/src/stfs/StfsContainer.cpp
    ${GXBUILD3_ROOT}/src/utils/FileManager.cpp
    ${GXBUILD3_ROOT}/src/utils/FusesetGenerator.cpp
    ${GXBUILD3_ROOT}/src/utils/Log.cpp
    ${GXBUILD3_ROOT}/src/utils/Options.cpp
    ${GXBUILD3_ROOT}/src/utils/Utils.cpp
)

add_library(gxbuild3_lib STATIC ${GXBUILD3_LIB_SOURCES})
target_compile_features(gxbuild3_lib PRIVATE cxx_std_23)
target_include_directories(gxbuild3_lib PUBLIC
    ${GXBUILD3_ROOT}/include
)
target_compile_definitions(gxbuild3_lib PRIVATE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)
target_link_libraries(gxbuild3_lib PUBLIC
    GxCrypt
    spdlog::spdlog
    stfs
)
