SUMMARY = "A small image just capable of allowing a device to boot."
LICENSE = "MIT"

inherit core-image

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = " "

# Set rootfs to 700MiB:
IMAGE_OVERHEAD_FACTOR = "2.0"
IMAGE_ROOTFS_SIZE = "716800"
IMAGE_ROOTFS_EXTRA_SPACE:append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "", d)}"


# Tools:
ZIGBEE2MQTT = "nodejs nodejs-npm make git gcc g++ bash curl lsb-release ca-certificates"
MOSQUITTO = "libmosquitto1 libmosquittopp1 mosquitto mosquitto-clients"

IMAGE_INSTALL:append = " i2c-tools apt apt-openstlinux ${ZIGBEE2MQTT} ${MOSQUITTO}"



# Kernel and RootFS:

# Apps:
IMAGE_INSTALL:append = " bmplcd"


# Services:
IMAGE_INSTALL:append = " temperature-systemd"


# Modules:
IMAGE_INSTALL:append = " mygpio-mod"
MACHINE_ESSENTIAL_EXTRA_RDEPENDS += "kernel-module-mygpio"

IMAGE_INSTALL:append = " lcd1602-mod"
MACHINE_ESSENTIAL_EXTRA_RDEPENDS += "kernel-module-lcd1602"

