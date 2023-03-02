FILESEXTRAPATHS:prepend := "${THISDIR}:${THISDIR}/files:"
SRC_URI += "file://02-add-dtsprobe.patch"

# Apply default configuration
SRC_URI += "file://defconfig"
KERNEL_DEFCONFIG_stm32mp1 = "${WORKDIR}/defconfig"

# Prevent the use of in-tree defconfig
unset KBUILD_DEFCONFIG
