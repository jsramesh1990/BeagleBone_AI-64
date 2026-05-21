SUMMARY = "Platform core package group"
DESCRIPTION = "Common packages for all platform images"
LICENSE = "MIT"

inherit packagegroup

PACKAGES = "\
    packagegroup-platform-core \
    packagegroup-platform-network \
    packagegroup-platform-storage \
    packagegroup-platform-monitoring \
"

RDEPENDS:packagegroup-platform-core = "\
    kernel-modules \
    bash \
    coreutils \
    util-linux \
    systemd \
    udev \
    openssh \
    ca-certificates \
    ntp \
"

RDEPENDS:packagegroup-platform-network = "\
    networkmanager \
    iptables \
    bridge-utils \
    curl \
    wget \
"

RDEPENDS:packagegroup-platform-storage = "\
    e2fsprogs \
    dosfstools \
    parted \
    lvm2 \
"

RDEPENDS:packagegroup-platform-monitoring = "\
    htop \
    iotop \
    sysstat \
    lm-sensors \
"
