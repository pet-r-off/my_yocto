SUMMARY = "Add ziqbee2mqtt to your image"
LICENSE = "MIT"

ZIGBEE2MQTT = "nodejs nodejs-npm make git gcc g++ bash curl lsb-release ca-certificates apt apt-openstlinux"
MOSQUITTO = "libmosquitto1 libmosquittopp1 mosquitto mosquitto-clients"

DEPENDS = "${ZIGBEE2MQTT} ${MOSQUITTO}"
RDEPENDS:${PN} = "${ZIGBEE2MQTT} ${MOSQUITTO}"

SRC_URI = "file://zigbee2mqtt.zip"

S = "${WORKDIR}"

# Install in directory /home/root
do_install() {
    install -d ${D}/home/root
    install -m 0777 ${WORKDIR}/zigbee2mqtt.zip ${D}/home/root
}

# Pack the path
FILES:${PN} += "/home/root"
