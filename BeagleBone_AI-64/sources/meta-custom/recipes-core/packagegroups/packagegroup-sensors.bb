# Package group for sensor support

SUMMARY = "Sensors Package Group"
LICENSE = "MIT"

inherit packagegroup

# ============================================================================
# Package Definitions
# ============================================================================
PACKAGES = " \
    packagegroup-sensors \
    packagegroup-sensors-core \
    packagegroup-sensors-drivers \
    packagegroup-sensors-utils \
    "

# ============================================================================
# Sensor Packages
# ============================================================================
RDEPENDS:packagegroup-sensors = " \
    packagegroup-sensors-core \
    packagegroup-sensors-drivers \
    packagegroup-sensors-utils \
    "

RDEPENDS:packagegroup-sensors-core = " \
    sensor-service \
    device-manager \
    iio-sensor-proxy \
    "

RDEPENDS:packagegroup-sensors-drivers = " \
    kernel-module-mpu6050 \
    kernel-module-bmp180 \
    kernel-module-dht11 \
    kernel-module-htu21d \
    kernel-module-gps \
    "

RDEPENDS:packagegroup-sensors-utils = " \
    i2c-tools \
    spi-tools \
    gpio-tools \
    gpsd \
    gpsd-clients \
    "

# ============================================================================
# End of packagegroup-sensors.bb
# ============================================================================
