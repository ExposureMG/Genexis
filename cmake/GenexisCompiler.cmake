# Select defaults before project() enables C and C++. Explicit selections and
# toolchain files belong to CMake; never overwrite a compiler in an existing build.
function(genexis_select_compilers)
    if(CMAKE_C_COMPILER OR CMAKE_CXX_COMPILER OR CMAKE_TOOLCHAIN_FILE
       OR NOT "$ENV{CC}" STREQUAL "" OR NOT "$ENV{CXX}" STREQUAL ""
       OR NOT "$ENV{CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        return()
    endif()

    # Require a complete pair so discovery cannot mix Clang and GCC.
    find_program(_genexis_clang NAMES clang NO_CACHE)
    find_program(_genexis_clangxx NAMES clang++ NO_CACHE)
    if(_genexis_clang AND _genexis_clangxx)
        set(CMAKE_C_COMPILER "${_genexis_clang}" CACHE FILEPATH "C compiler")
        set(CMAKE_CXX_COMPILER "${_genexis_clangxx}" CACHE FILEPATH "C++ compiler")
        return()
    endif()

    find_program(_genexis_gcc NAMES gcc NO_CACHE)
    find_program(_genexis_gxx NAMES g++ NO_CACHE)
    if(_genexis_gcc AND _genexis_gxx)
        set(CMAKE_C_COMPILER "${_genexis_gcc}" CACHE FILEPATH "C compiler")
        set(CMAKE_CXX_COMPILER "${_genexis_gxx}" CACHE FILEPATH "C++ compiler")
        return()
    endif()

    if(CMAKE_HOST_WIN32 OR WIN32)
        find_program(_genexis_cl NAMES cl NO_CACHE)
        if(_genexis_cl OR CMAKE_GENERATOR MATCHES "Visual Studio")
            return()
        endif()
    endif()

    message(FATAL_ERROR
        "Genexis requires Clang (clang and clang++), GCC (gcc and g++), or MSVC (cl). "
        "Install a supported compiler on PATH or specify a toolchain.")
endfunction()

function(genexis_validate_compilers)
    foreach(_genexis_language IN ITEMS C CXX)
        if(NOT CMAKE_${_genexis_language}_COMPILER_ID MATCHES "^(Clang|AppleClang|GNU|MSVC)$")
            message(FATAL_ERROR
                "Genexis requires Clang, GCC, or MSVC for ${_genexis_language}; "
                "detected '${CMAKE_${_genexis_language}_COMPILER_ID}'.")
        endif()
    endforeach()
endfunction()
