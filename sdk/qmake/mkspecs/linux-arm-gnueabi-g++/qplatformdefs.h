/****************************************************************************
**
** Copyright (C) 2024 BBB AI-64 Project
** Contact: https://github.com/yourusername/BeagleBone_AI-64
**
** This file is part of the qmake configuration for BBB AI-64.
**
****************************************************************************/

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

// Get the standard platform definitions
#include "../linux-g++/qplatformdefs.h"

// ============================================================================
// ARM-Specific Definitions
// ============================================================================

// Cache line size for ARM Cortex-A72
#define QT_CACHE_LINE_SIZE 64

// ARM architecture features
#define QT_ARCH_ARM 1
#define QT_HAVE_NEON 1

// ============================================================================
// Endianness
// ============================================================================
#if defined(__ARMEL__) || defined(__ARM_LITTLE_ENDIAN__)
#define Q_BYTE_ORDER Q_LITTLE_ENDIAN
#else
#define Q_BYTE_ORDER Q_BIG_ENDIAN
#endif

// ============================================================================
// POSIX Threads
// ============================================================================
#ifndef QT_NO_THREAD
#define QT_USE_PTHREADS
#endif

// ============================================================================
// Standard Library
// ============================================================================
#define QT_USE_GLIBC

// ============================================================================
// End of qplatformdefs.h
// ============================================================================
