SUMMARY = "Security hardening configuration"
LICENSE = "MIT"

inherit systemd

SRC_URI = " \
    file://99-security.conf \
    file://security-hardening.service \
"

do_install() {
    # Security sysctl
    install -d ${D}${sysconfdir}/sysctl.d
    install -m 0644 ${WORKDIR}/99-security.conf ${D}${sysconfdir}/sysctl.d/
    
    # Service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/security-hardening.service ${D}${systemd_system_unitdir}/
}

SYSTEMD_SERVICE:${PN} = "security-hardening.service"
