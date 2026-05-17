cmake_minimum_required(VERSION 3.10)
project(FTDI2SPI)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libmpsse/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libmpsse/libftd2xx
)

set(SRCS
    Main.cpp
    wrapper_spi.cpp
    Flasher.cpp
    sfc.cpp
    XSPI.cpp
    XNAND.cpp
    XeMMC.cpp
    ISD2100.cpp
)

add_library(ftdi2spi SHARED ${SRCS})
target_link_directories(ftdi2spi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libmpsse/build)
target_link_libraries(ftdi2spi PRIVATE mpsse pthread dl)

add_executable(ftdi2spi_bin ${SRCS})
set_target_properties(ftdi2spi_bin PROPERTIES OUTPUT_NAME "ftdi2spi")
target_link_directories(ftdi2spi_bin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libmpsse/build)
target_link_libraries(ftdi2spi_bin PRIVATE mpsse pthread dl)
