SUMMARY = "OTA update support"
LICENSE = "MIT"

inherit packagegroup

PACKAGES = "packagegroup-ota"

RDEPENDS:packagegroup-ota = "\
    rauc \
    rauc-hawkbit-updater \
    rauc-service \
    grub \
    u-boot-fw-utils \
    efibootmgr \
"
