# Custom image recipe for the BeagleBone AI-64 project

DESCRIPTION = "Complete custom image for BeagleBone AI-64 with GUI and sensors"

# ============================================================================
# Image Type
# ============================================================================
require recipes-core/images/core-image-base.bb

# ============================================================================
# Image Features
# ============================================================================
IMAGE_FEATURES:append = " \
    ssh-server-openssh \
    package-management \
    hwcodecs \
    \
    ${@bb.utils.contains('MACHINE_FEATURES', 'display', 'x11-base', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'sensors', '', d)} \
    "

# ============================================================================
# Package Groups
# ============================================================================
IMAGE_INSTALL:append = " \
    packagegroup-core-full-cmdline \
    packagegroup-base-extended \
    ${@bb.utils.contains('MACHINE_FEATURES', 'display', 'packagegroup-gui', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'sensors', 'packagegroup-sensors', '', d)} \
    packagegroup-system \
    "

# ============================================================================
# Individual Packages
# ============================================================================
IMAGE_INSTALL:append = " \
    i2c-tools \
    spi-tools \
    gpio-tools \
    usbutils \
    pciutils \
    network-tools \
    \
    python3 \
    python3-pip \
    python3-serial \
    python3-json \
    \
    git \
    vim \
    tmux \
    htop \
    iotop \
    \
    connman \
    connman-client \
    networkmanager \
    \
    openssl \
    openssh \
    ca-certificates \
    \
    docker \
    docker-compose \
    containerd \
    runc \
    \
    qtbase \
    qtdeclarative \
    qtquickcontrols2 \
    qtsensors \
    qtserialport \
    qtwebsockets \
    qtmultimedia \
    qtcharts \
    qtvirtualkeyboard \
    \
    sensor-service \
    device-manager \
    gui-app \
    updater \
    logger \
    ipc \
    config \
    utils \
    "

# ============================================================================
# Development Tools (for development image only)
# ============================================================================
IMAGE_INSTALL:append = " \
    ${@bb.utils.contains('IMAGE_FEATURES', 'tools-debug', 'gdb strace', '', d)} \
    ${@bb.utils.contains('IMAGE_FEATURES', 'tools-profile', 'perf oprofile', '', d)} \
    ${@bb.utils.contains('IMAGE_FEATURES', 'dev-pkgs', 'gcc make cmake', '', d)} \
    "

# ============================================================================
# System Services
# ============================================================================
SYSTEMD_SERVICE:${PN} = " \
    connman.service \
    sshd.service \
    docker.service \
    sensor-service.service \
    device-manager.service \
    gui-app.service \
    "

# ============================================================================
# End of custom-image.bb
# ============================================================================
