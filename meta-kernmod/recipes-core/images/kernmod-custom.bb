SUMMARY = "A small image just capable of allowing a device to boot."
LICENSE = "MIT"

inherit core-image

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = " "

# Set rootfs to 2GiB:
IMAGE_OVERHEAD_FACTOR = "1.0"
IMAGE_ROOTFS_SIZE = "2097152"


# Tools:
IMAGE_INSTALL:append = " i2c-tools zigbee2mqtt"



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

