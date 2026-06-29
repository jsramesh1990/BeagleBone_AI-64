# Package group for IoT support
# This package group includes IoT protocols and tools

SUMMARY = "IoT Package Group"
DESCRIPTION = "Package group for IoT protocols and tools"
LICENSE = "MIT"

inherit packagegroup

PACKAGES = "\
    packagegroup-iot \
    packagegroup-iot-protocols \
    packagegroup-iot-tools \
    "

RDEPENDS:packagegroup-iot = "\
    packagegroup-iot-protocols \
    packagegroup-iot-tools \
    "

# IoT Protocols
RDEPENDS:packagegroup-iot-protocols = "\
    mosquitto \
    mosquitto-clients \
    paho-mqtt-c \
    paho-mqtt-cpp \
    libcoap \
    libcoap-bin \
    can-utils \
    "

# IoT Tools
RDEPENDS:packagegroup-iot-tools = "\
    python3-paho-mqtt \
    python3-opcua \
    python3-modbus \
    nodejs \
    npm \
    "

SUMMARY:packagegroup-iot = "IoT Package Group"
SUMMARY:packagegroup-iot-protocols = "IoT Protocols"
SUMMARY:packagegroup-iot-tools = "IoT Tools"

# ============================================================================
# End of packagegroup-iot.bb
# ============================================================================
