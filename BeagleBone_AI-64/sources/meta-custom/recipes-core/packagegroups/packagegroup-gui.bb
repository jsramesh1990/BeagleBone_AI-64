# Package group for GUI applications

SUMMARY = "GUI Package Group"
LICENSE = "MIT"

inherit packagegroup

# ============================================================================
# Package Definitions
# ============================================================================
PACKAGES = " \
    packagegroup-gui \
    packagegroup-gui-qt5 \
    packagegroup-gui-utils \
    "

# ============================================================================
# GUI Packages
# ============================================================================
RDEPENDS:packagegroup-gui = " \
    packagegroup-gui-qt5 \
    packagegroup-gui-utils \
    "

RDEPENDS:packagegroup-gui-qt5 = " \
    qtbase \
    qtdeclarative \
    qtquickcontrols2 \
    qtsensors \
    qtserialport \
    qtwebsockets \
    qtmultimedia \
    qtcharts \
    qtvirtualkeyboard \
    "

RDEPENDS:packagegroup-gui-utils = " \
    gui-app \
    dashboard \
    qt-launcher \
    "

# ============================================================================
# End of packagegroup-gui.bb
# ============================================================================
