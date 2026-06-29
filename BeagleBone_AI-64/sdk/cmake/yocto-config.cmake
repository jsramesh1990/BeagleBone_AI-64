# yocto-config.cmake - Yocto Build Configuration
# This file configures CMake for Yocto cross-compilation

# ============================================================================
# Yocto Configuration
# ============================================================================

# Set the Yocto SDK root
if(NOT DEFINED Yocto_ROOT)
    if(DEFINED ENV{YOCTO_ROOT})
        set(Yocto_ROOT $ENV{YOCTO_ROOT})
    else()
        set(Yocto_ROOT "/opt/poky/1.0")
    endif()
endif()

# Set the target architecture
if(NOT DEFINED Yocto_ARCH)
    set(Yocto_ARCH "cortexa72")
endif()

# Set the target triplet
set(Yocto_TRIPLET "${Yocto_ARCH}-poky-linux")

# ============================================================================
# Toolchain Paths
# ============================================================================
set(Yocto_TOOLCHAIN_DIR "${Yocto_ROOT}/sysroots/x86_64-pokysdk-linux/usr/bin/${Yocto_TRIPLET}")
set(Yocto_SYSROOT "${Yocto_ROOT}/sysroots/${Yocto_TRIPLET}")

# ============================================================================
# Compiler Settings
# ============================================================================
set(CMAKE_C_COMPILER "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-gcc")
set(CMAKE_CXX_COMPILER "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-g++")
set(CMAKE_AR "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-ar")
set(CMAKE_LINKER "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-ld")
set(CMAKE_STRIP "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-strip")
set(CMAKE_OBJCOPY "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-objcopy")
set(CMAKE_OBJDUMP "${Yocto_TOOLCHAIN_DIR}/${Yocto_TRIPLET}-objdump")

# ============================================================================
# Sysroot Settings
# ============================================================================
set(CMAKE_SYSROOT ${Yocto_SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${Yocto_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ============================================================================
# Compiler Flags
# ============================================================================
set(CMAKE_C_FLAGS_INIT "-O2 -pipe -g -feliminate-unused-debug-types")
set(CMAKE_CXX_FLAGS_INIT "-O2 -pipe -g -feliminate-unused-debug-types")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-rpath-link,${Yocto_SYSROOT}/usr/lib")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,-rpath-link,${Yocto_SYSROOT}/usr/lib")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-Wl,-rpath-link,${Yocto_SYSROOT}/usr/lib")

# ============================================================================
# pkg-config Settings
# ============================================================================
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${Yocto_SYSROOT})
set(ENV{PKG_CONFIG_PATH} "${Yocto_SYSROOT}/usr/lib/pkgconfig:${Yocto_SYSROOT}/usr/share/pkgconfig")

# ============================================================================
# Qt6 Settings
# ============================================================================
find_program(Yocto_QMAKE
    NAMES qmake
    PATHS ${Yocto_TOOLCHAIN_DIR}/../bin
)

if(Yocto_QMAKE)
    set(QT_QMAKE_EXECUTABLE ${Yocto_QMAKE})
    set(CMAKE_PREFIX_PATH ${Yocto_SYSROOT}/usr)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
endif()

# ============================================================================
# End of yocto-config.cmake
# ============================================================================
