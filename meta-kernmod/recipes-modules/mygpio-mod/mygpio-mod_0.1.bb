SUMMARY = "Example of how to build an external Linux kernel module"
DESCRIPTION = "${SUMMARY}"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"


inherit module


SRC_URI = "file://Makefile \
           file://mygpio.c \
           file://COPYING \
          "


S = "${WORKDIR}"


# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

RPROVIDES_${PN} += "kernel-module-mygpio"


# This line allows to automatically load module in-tree 
# P.S. Doesnt work in local.conf, only in recipe

KERNEL_MODULE_AUTOLOAD = "mygpio" 


# To include module to the image add lines to kernmod-custom.bb ->
# IMAGE_INSTALL:append = " mygpio-mod"   ->  name of *.bb recipe
# MACHINE_ESSENTIAL_EXTRA_RDEPENDS += "kernel-module-mygpio" ->  name of LKM module
