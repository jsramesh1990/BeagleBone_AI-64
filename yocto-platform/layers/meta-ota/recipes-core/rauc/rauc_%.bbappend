# layers/meta-ota/recipes-core/rauc/rauc_%.bbappend
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://platform.cert.pem \
    file://ca.cert.pem \
    file://system.conf \
"

do_install:append() {
    install -d ${D}${sysconfdir}/rauc
    install -m 0644 ${WORKDIR}/system.conf ${D}${sysconfdir}/rauc/
}
