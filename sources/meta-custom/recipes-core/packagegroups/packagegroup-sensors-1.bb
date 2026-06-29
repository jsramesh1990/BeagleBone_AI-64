# Package group for sensor support
# This package group includes all necessary components for sensor functionality

SUMMARY = "Sensors Package Group"
DESCRIPTION = "Package group for sensor drivers, services, and utilities"
LICENSE = "MIT"

inherit packagegroup

# ============================================================================
# Package Definitions
# ============================================================================
PACKAGES = "\
    packagegroup-sensors \
    packagegroup-sensors-core \
    packagegroup-sensors-drivers \
    packagegroup-sensors-utils \
    packagegroup-sensors-services \
    "

# ============================================================================
# Runtime Dependencies
# ============================================================================
RDEPENDS:packagegroup-sensors = "\
    packagegroup-sensors-core \
    packagegroup-sensors-drivers \
    packagegroup-sensors-utils \
    packagegroup-sensors-services \
    "

# ============================================================================
# Core Sensor Packages
# ============================================================================
RDEPENDS:packagegroup-sensors-core = "\
    sensor-service \
    device-manager \
    iio-sensor-proxy \
    libsensors \
    libsensors-dev \
    "

# ============================================================================
# Sensor Drivers
# ============================================================================
RDEPENDS:packagegroup-sensors-drivers = "\
    kernel-module-i2c-dev \
    kernel-module-spidev \
    kernel-module-gpio-mxc \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'kernel-module-mpu6050', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'kernel-module-bmp180', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'kernel-module-dht11', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'kernel-module-htu21d', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'kernel-module-gps', '', d)} \
    "

# ============================================================================
# Sensor Utilities
# ============================================================================
RDEPENDS:packagegroup-sensors-utils = "\
    i2c-tools \
    i2c-tools-dev \
    spi-tools \
    gpio-tools \
    gpsd \
    gpsd-clients \
    gpsd-gpsctl \
    "

# ============================================================================
# Sensor Services
# ============================================================================
RDEPENDS:packagegroup-sensors-services = "\
    sensor-service \
    sensor-service-conf \
    sensor-service-systemd \
    "

# ============================================================================
# Additional Sensor Types
# ============================================================================
RDEPENDS:packagegroup-sensors:append = "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'camera', 'packagegroup-sensors-camera', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'audio', 'packagegroup-sensors-audio', '', d)} \
    "

# ============================================================================
# Camera Sensors
# ============================================================================
RDEPENDS:packagegroup-sensors-camera = "\
    kernel-module-ov5640 \
    kernel-module-imx219 \
    media-ctl \
    v4l-utils \
    "

# ============================================================================
# Audio Sensors
# ============================================================================
RDEPENDS:packagegroup-sensors-audio = "\
    alsa-utils \
    alsa-tools \
    kernel-module-snd-soc-tlv320aic3x \
    "

# ============================================================================
# I2C/SPI Support
# ============================================================================
RDEPENDS:packagegroup-sensors:append = "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'i2c', 'packagegroup-sensors-i2c', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'spi', 'packagegroup-sensors-spi', '', d)} \
    "

RDEPENDS:packagegroup-sensors-i2c = "\
    i2c-tools \
    python3-i2c-tools \
    "

RDEPENDS:packagegroup-sensors-spi = "\
    spi-tools \
    python3-spi-tools \
    "

# ============================================================================
# Python Bindings
# ============================================================================
RDEPENDS:packagegroup-sensors:append = "\
    python3-serial \
    python3-smbus \
    python3-pyserial \
    python3-pyserial-asyncio \
    "

# ============================================================================
# Configuration Files
# ============================================================================
CONFFILES:packagegroup-sensors:append = "\
    /etc/sensor-service/config.json \
    /etc/device-manager/config.json \
    "

# ============================================================================
# Summary
# ============================================================================
SUMMARY:packagegroup-sensors = "Sensors Package Group"
SUMMARY:packagegroup-sensors-core = "Core Sensor Components"
SUMMARY:packagegroup-sensors-drivers = "Sensor Drivers"
SUMMARY:packagegroup-sensors-utils = "Sensor Utilities"
SUMMARY:packagegroup-sensors-services = "Sensor Services"
SUMMARY:packagegroup-sensors-camera = "Camera Sensors"
SUMMARY:packagegroup-sensors-audio = "Audio Sensors"

# ============================================================================
# End of packagegroup-sensors.bb
# ============================================================================
