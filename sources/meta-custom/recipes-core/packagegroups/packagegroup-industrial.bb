# Package group for Industrial IoT support
# This package group includes industrial protocols and tools

SUMMARY = "Industrial IoT Package Group"
DESCRIPTION = "Package group for industrial protocols and tools"
LICENSE = "MIT"

inherit packagegroup

PACKAGES = "\
    packagegroup-industrial \
    packagegroup-industrial-protocols \
    packagegroup-industrial-tools \
    "

RDEPENDS:packagegroup-industrial = "\
    packagegroup-industrial-protocols \
    packagegroup-industrial-tools \
    "

# Industrial Protocols
RDEPENDS:packagegroup-industrial-protocols = "\
    opcua \
    opcua-bin \
    modbus \
    modbus-tools \
    can-utils \
    """

# Industrial Tools
RDEPENDS:packagegroup-industrial-tools = "\
    python3-opcua \
    python3-modbus \
    python3-pyads \
    mqtt-sparkplug \
    "

SUMMARY:packagegroup-industrial = "Industrial IoT Package Group"
SUMMARY:packagegroup-industrial-protocols = "Industrial Protocols"
SUMMARY:packagegroup-industrial-tools = "Industrial Tools"

# ============================================================================
# End of packagegroup-industrial.bb
# ============================================================================
