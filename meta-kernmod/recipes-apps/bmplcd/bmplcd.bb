SUMMARY = "BMP180 to LCD1602 pyhton application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "python3"
RDEPENDS:${PN} = "python3"

SRC_URI = "file://bmplcd.py"

S = "${WORKDIR}"

# Install app directory /home/root
do_install() {
    install -d ${D}/home/root
    install -m 0755 ${WORKDIR}/bmplcd.py ${D}/home/root
}

# Pack the path
FILES:${PN} += "/home/root"
