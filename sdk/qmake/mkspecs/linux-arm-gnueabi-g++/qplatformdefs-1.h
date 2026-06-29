/****************************************************************************
**
** Copyright (C) 2024 BBB AI-64 Project
** Contact: https://github.com/yourusername/BeagleBone_AI-64
**
** This file is part of the qmake configuration for cross-compilation
** targeting the BeagleBone Black AI-64 platform.
**
** Commercial License Usage:
** Licensees holding valid commercial licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and the BBB AI-64 Project.
**
** GNU General Public License Usage:
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or later as published by the Free
** Software Foundation and appearing in the file LICENSE included in
** the packaging of this file.
**
****************************************************************************/

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

// ============================================================================
// Standard Platform Definitions
// ============================================================================
// Get the standard Linux/GCC platform definitions
#include "../linux-g++/qplatformdefs.h"

// ============================================================================
// ARM-Specific Definitions
// ============================================================================
// Cache line size for ARM Cortex-A72 (64 bytes)
#define QT_CACHE_LINE_SIZE 64

// ARM architecture features
#define QT_ARCH_ARM 1
#define QT_HAVE_NEON 1
#define QT_HAVE_ARM_NEON 1

// ARM architecture version
#define QT_ARCH_ARM_V7A 1
#define QT_ARCH_ARM_V8A 1

// ============================================================================
// Endianness
// ============================================================================
#if defined(__ARMEL__) || defined(__ARM_LITTLE_ENDIAN__)
#define Q_BYTE_ORDER Q_LITTLE_ENDIAN
#else
#define Q_BYTE_ORDER Q_BIG_ENDIAN
#endif

// ============================================================================
// Threading Support
// ============================================================================
#ifndef QT_NO_THREAD
#define QT_USE_PTHREADS
#define QT_PTHREAD_SETNAME_NP
#endif

// ============================================================================
// Standard Library
// ============================================================================
#define QT_USE_GLIBC
#define QT_GLIBC_VERSION 2.36

// ============================================================================
// Floating Point
// ============================================================================
#define QT_USE_SOFT_FLOAT 0
#define QT_USE_HARD_FLOAT 1

// ============================================================================
// Kernel Features
// ============================================================================
#define QT_SYSINFO_EPOCH 1
#define QT_SYSINFO_CPU 1
#define QT_SYSINFO_LOADAVG 1

// ============================================================================
// End of qplatformdefs.h
// ============================================================================
