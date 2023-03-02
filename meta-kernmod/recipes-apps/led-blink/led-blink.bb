SUMMARY = "STM32 led blink"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"


# hello-world.c from local file
SRC_URI = "file://led-blink.cpp"

# Set LDFLAGS options provided by the build system
TARGET_CC_ARCH += "${LDFLAGS}"
 
# Change source directory to workdirectory where led-blink.cpp is
S = "${WORKDIR}"
 
# Compile hello-world from sources, no Makefile
do_compile() {
    ${CXX} -Wall led-blink.cpp -o led-blink
}
 
# Install binary to final directory /usr/bin
do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/led-blink ${D}${bindir}
}
