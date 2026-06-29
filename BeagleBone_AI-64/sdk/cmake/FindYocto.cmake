# FindYocto.cmake - Find Yocto SDK components
# This module finds the Yocto SDK and sets up the environment for cross-compilation
#
# This module defines the following variables:
#   Yocto_FOUND          - True if Yocto SDK found
#   Yocto_SYSROOT        - Path to the Yocto sysroot
#   Yocto_TOOLCHAIN_DIR  - Path to the toolchain directory
#   Yocto_C_COMPILER     - C compiler path
#   Yocto_CXX_COMPILER   - C++ compiler path
#   Yocto_VERSION        - Yocto version
#   Yocto_ARCH           - Target architecture
#
# This module defines the following imported targets:
#   Yocto::Toolchain     - Target for using the Yocto toolchain

# ============================================================================
# Find Yocto SDK
# ============================================================================

# Search for Yocto SDK in common locations
find_path(Yocto_ROOT
    NAMES environment-setup-cortexa72-poky-linux
    PATHS
        /opt/poky
        /opt/yocto
        /usr/local/poky
        $ENV{HOME}/yocto
        $ENV{YOCTO_DIR}
    DOC "Yocto SDK root directory"
)

# If Yocto_ROOT is found, set up the environment
if(Yocto_ROOT)
    # Find the sysroot
    find_path(Yocto_SYSROOT
        NAMES usr/include/stdio.h
        PATHS
            ${Yocto_ROOT}/sysroots/cortexa72-poky-linux
            ${Yocto_ROOT}/sysroots/aarch64-poky-linux
        DOC "Yocto sysroot directory"
    )

    # Find the toolchain directory
    find_path(Yocto_TOOLCHAIN_DIR
        NAMES arm-poky-linux-gnueabi-gcc
        PATHS
            ${Yocto_ROOT}/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi
            ${Yocto_ROOT}/sysroots/x86_64-pokysdk-linux/usr/bin
        DOC "Yocto toolchain directory"
    )

    # Find the compiler
    find_program(Yocto_C_COMPILER
        NAMES arm-poky-linux-gnueabi-gcc arm-poky-linux-gnueabi-gcc
        PATHS
            ${Yocto_TOOLCHAIN_DIR}
        DOC "Yocto C compiler"
    )

    find_program(Yocto_CXX_COMPILER
        NAMES arm-poky-linux-gnueabi-g++ arm-poky-linux-gnueabi-g++
        PATHS
            ${Yocto_TOOLCHAIN_DIR}
        DOC "Yocto C++ compiler"
    )

    # Find other tools
    find_program(Yocto_AR
        NAMES arm-poky-linux-gnueabi-ar
        PATHS ${Yocto_TOOLCHAIN_DIR}
    )

    find_program(Yocto_LD
        NAMES arm-poky-linux-gnueabi-ld
        PATHS ${Yocto_TOOLCHAIN_DIR}
    )

    find_program(Yocto_STRIP
        NAMES arm-poky-linux-gnueabi-strip
        PATHS ${Yocto_TOOLCHAIN_DIR}
    )

    find_program(Yocto_OBJCOPY
        NAMES arm-poky-linux-gnueabi-objcopy
        PATHS ${Yocto_TOOLCHAIN_DIR}
    )

    find_program(Yocto_GDB
        NAMES arm-poky-linux-gnueabi-gdb
        PATHS ${Yocto_TOOLCHAIN_DIR}
    )

    # Determine version
    if(Yocto_C_COMPILER)
        execute_process(
            COMMAND ${Yocto_C_COMPILER} --version
            OUTPUT_VARIABLE _compiler_version
            ERROR_QUIET
        )
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" Yocto_VERSION "${_compiler_version}")
        if(NOT Yocto_VERSION)
            set(Yocto_VERSION "1.0.0")
        endif()
    endif()

    # Determine architecture
    if(Yocto_SYSROOT)
        if(EXISTS "${Yocto_SYSROOT}/usr/lib/aarch64-linux-gnu")
            set(Yocto_ARCH "aarch64")
        elseif(EXISTS "${Yocto_SYSROOT}/usr/lib/arm-linux-gnueabi")
            set(Yocto_ARCH "arm")
        else()
            set(Yocto_ARCH "unknown")
        endif()
    endif()

    # Set the toolchain variables
    set(CMAKE_C_COMPILER ${Yocto_C_COMPILER})
    set(CMAKE_CXX_COMPILER ${Yocto_CXX_COMPILER})
    set(CMAKE_SYSROOT ${Yocto_SYSROOT})
    set(CMAKE_FIND_ROOT_PATH ${Yocto_SYSROOT})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    # Set compiler flags
    set(CMAKE_C_FLAGS "-O2 -pipe -g")
    set(CMAKE_CXX_FLAGS "-O2 -pipe -g")
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath-link,${Yocto_SYSROOT}/usr/lib")

    # Set pkg-config
    set(ENV{PKG_CONFIG_SYSROOT_DIR} ${Yocto_SYSROOT})
    set(ENV{PKG_CONFIG_PATH} "${Yocto_SYSROOT}/usr/lib/pkgconfig:${Yocto_SYSROOT}/usr/share/pkgconfig")

    # Set Qt6
    find_program(QT_QMAKE_EXECUTABLE
        NAMES qmake
        PATHS ${Yocto_TOOLCHAIN_DIR}/../bin
    )
    if(QT_QMAKE_EXECUTABLE)
        set(CMAKE_PREFIX_PATH ${Yocto_SYSROOT}/usr)
    endif()

    # Create imported target
    add_library(Yocto::Toolchain INTERFACE IMPORTED)
    set_target_properties(Yocto::Toolchain PROPERTIES
        INTERFACE_COMPILE_OPTIONS "-O2 -pipe"
        INTERFACE_LINK_OPTIONS "-Wl,-rpath-link,${Yocto_SYSROOT}/usr/lib"
        INTERFACE_INCLUDE_DIRECTORIES "${Yocto_SYSROOT}/usr/include"
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${Yocto_SYSROOT}/usr/include"
    )

    set(Yocto_FOUND TRUE)
else()
    set(Yocto_FOUND FALSE)
endif()

# ============================================================================
# Handle find_package requirements
# ============================================================================
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Yocto
    REQUIRED_VARS
        Yocto_ROOT
        Yocto_SYSROOT
        Yocto_TOOLCHAIN_DIR
        Yocto_C_COMPILER
        Yocto_CXX_COMPILER
    VERSION_VAR Yocto_VERSION
)

# ============================================================================
# Mark variables as advanced
# ============================================================================
mark_as_advanced(
    Yocto_ROOT
    Yocto_SYSROOT
    Yocto_TOOLCHAIN_DIR
    Yocto_C_COMPILER
    Yocto_CXX_COMPILER
    Yocto_AR
    Yocto_LD
    Yocto_STRIP
    Yocto_OBJCOPY
    Yocto_GDB
    Yocto_VERSION
    Yocto_ARCH
)

# ============================================================================
# End of FindYocto.cmake
# ============================================================================
