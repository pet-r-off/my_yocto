SUMMARY = "Install and start a systemd service"
LICENSE = "CLOSED"

inherit systemd
inherit features_check

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "temperature.service"

SRC_URI:append = " file://temperature.service "
FILES:${PN} += "${systemd_unitdir}/system/temperature.service"

S = "${WORKDIR}"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/temperature.service ${D}/${systemd_unitdir}/system
}

# Pack the path
FILES:${PN} += "/lib/systemd/system"

REQUIRED_DISTRO_FEATURES= "systemd"
