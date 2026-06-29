# BBB AI-64 CMake Toolchain File
# This file configures CMake for cross-compilation

# ============================================================================
# System Information
# ============================================================================
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)

# ============================================================================
# Toolchain Paths
# ============================================================================
set(SDK_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
set(TOOLCHAIN_DIR ${SDK_DIR}/toolchain)
set(SYSROOT ${TOOLCHAIN_DIR}/sysroot)

# Cross-compiler prefix
set(CROSS_COMPILE arm-poky-linux-gnueabi-)

# ============================================================================
# Compiler Settings
# ============================================================================
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}as)
set(CMAKE_AR ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}ar CACHE FILEPATH "Archiver")
set(CMAKE_LINKER ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}ld CACHE FILEPATH "Linker")
set(CMAKE_STRIP ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}strip CACHE FILEPATH "Strip")
set(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}objcopy CACHE FILEPATH "Objcopy")
set(CMAKE_OBJDUMP ${TOOLCHAIN_DIR}/bin/${CROSS_COMPILE}objdump CACHE FILEPATH "Objdump")

# ============================================================================
# Compiler Flags
# ============================================================================
set(CMAKE_C_FLAGS "-O2 -pipe -g -feliminate-unused-debug-types")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath-link,${SYSROOT}/usr/lib")

# ============================================================================
# Sysroot
# ============================================================================
set(CMAKE_SYSROOT ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ============================================================================
# pkg-config
# ============================================================================
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${SYSROOT})
set(ENV{PKG_CONFIG_PATH} "${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig")

# ============================================================================
# Qt6 Configuration
# ============================================================================
set(QT_QMAKE_EXECUTABLE ${TOOLCHAIN_DIR}/bin/qmake)
set(CMAKE_PREFIX_PATH ${SYSROOT}/usr)
find_package(Qt6 REQUIRED COMPONENTS Core Quick)

# ============================================================================
# Additional Settings
# ============================================================================
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SKIP_RPATH TRUE)

# ============================================================================
# End of Toolchain.cmake
# ============================================================================
