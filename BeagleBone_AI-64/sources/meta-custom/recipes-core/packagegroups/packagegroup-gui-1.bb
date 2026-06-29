# Package group for GUI applications
# This package group includes all necessary components for the graphical user interface

SUMMARY = "GUI Package Group"
DESCRIPTION = "Package group for GUI applications including Qt6 framework and dashboard"
LICENSE = "MIT"

inherit packagegroup

# ============================================================================
# Package Definitions
# ============================================================================
PACKAGES = "\
    packagegroup-gui \
    packagegroup-gui-qt6 \
    packagegroup-gui-apps \
    packagegroup-gui-utils \
    "

# ============================================================================
# Runtime Dependencies
# ============================================================================
RDEPENDS:packagegroup-gui = "\
    packagegroup-gui-qt6 \
    packagegroup-gui-apps \
    packagegroup-gui-utils \
    "

# ============================================================================
# Qt6 Framework Packages
# ============================================================================
RDEPENDS:packagegroup-gui-qt6 = "\
    qtbase \
    qtdeclarative \
    qtquickcontrols2 \
    qtquickcontrols \
    qtgraphicaleffects \
    qtmultimedia \
    qtsensors \
    qtserialport \
    qtwebsockets \
    qtcharts \
    qtvirtualkeyboard \
    qtimageformats \
    qtsvg \
    qtxmlpatterns \
    qttools \
    qtwayland \
    qtwayland-plugins \
    "

# ============================================================================
# GUI Applications
# ============================================================================
RDEPENDS:packagegroup-gui-apps = "\
    dashboard \
    qt-launcher \
    gui-app \
    "

# ============================================================================
# GUI Utilities
# ============================================================================
RDEPENDS:packagegroup-gui-utils = "\
    qtdemo \
    qtbase-examples \
    qtquickcontrols2-examples \
    "

# ============================================================================
# Additional Features for Display
# ============================================================================
RDEPENDS:packagegroup-gui:append = "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'display', 'packagegroup-gui-display', '', d)} \
    "

RDEPENDS:packagegroup-gui-display = "\
    weston \
    weston-conf \
    libdrm \
    libdrm-tests \
    mesa \
    mesa-driver-swrast \
    mesa-driver-virtio_gpu \
    "

# ============================================================================
# Touchscreen Support
# ============================================================================
RDEPENDS:packagegroup-gui:append = "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'touchscreen', 'packagegroup-gui-touch', '', d)} \
    "

RDEPENDS:packagegroup-gui-touch = "\
    tslib \
    tslib-conf \
    tslib-calibrate \
    tslib-tests \
    "

# ============================================================================
# Graphics Acceleration
# ============================================================================
RDEPENDS:packagegroup-gui:append = "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'gpu', 'packagegroup-gui-gpu', '', d)} \
    "

RDEPENDS:packagegroup-gui-gpu = "\
    libgles2-mesa \
    libgles2-mesa-dev \
    libgles2-mesa-dbg \
    "

# ============================================================================
# Fonts
# ============================================================================
RDEPENDS:packagegroup-gui:append = "\
    fontconfig \
    fontconfig-utils \
    ttf-dejavu-sans \
    ttf-dejavu-sans-mono \
    ttf-dejavu-serif \
    ttf-liberation \
    ttf-roboto \
    "

# ============================================================================
# Theme Support
# ============================================================================
RDEPENDS:packagegroup-gui:append = "\
    qtquickcontrols2-style-material \
    qtquickcontrols2-style-universal \
    adwaita-qt6 \
    "

# ============================================================================
# Summary
# ============================================================================
SUMMARY:packagegroup-gui = "GUI Package Group"
SUMMARY:packagegroup-gui-qt6 = "Qt6 Framework"
SUMMARY:packagegroup-gui-apps = "GUI Applications"
SUMMARY:packagegroup-gui-utils = "GUI Utilities"
SUMMARY:packagegroup-gui-display = "Display Server"
SUMMARY:packagegroup-gui-touch = "Touchscreen Support"
SUMMARY:packagegroup-gui-gpu = "GPU Acceleration"

# ============================================================================
# End of packagegroup-gui.bb
# ============================================================================
