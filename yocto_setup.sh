#!/bin/bash

# Update
sudo apt update
sudo apt upgrade


# Download dependencies
sudo apt-get install -y gcc git make net-tools libncurses5-dev tftpd zlib1g-dev \
			libssl-dev flex libsdl1.2-dev libglib2.0-dev zlib1g:i386 \
			screen pax gzip gawk bison libselinux1 gnupg wget diffstat \
			chrpath socat xterm autoconf libtool tar unzip texinfo zlib1g-dev \
			gcc-multilib build-essential curl

sudo apt-get install -y gawk wget git-core diffstat unzip texinfo gcc-multilib build-essential \
			chrpath socat cpio python python3 python3-pip python3-pexpect xz-utils \
			debianutils iputils-ping libsdl1.2-dev gdisk tree device-tree-compiler

sudo apt install -y gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio \
			python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping \
			python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev pylint3 xterm \
			python3-subunit mesa-common-dev zstd liblz4-tool


echo "alias python=python3" >> ~/.bashrc
python --version
source ~/.bashrc
python --version


sudo snap install code --classic


# Create project dir
mkdir ~/projects/
mkdir ~/projects/yocto/
cd ~/projects/yocto/


# Clone git repos
git clone -b kirkstone https://github.com/STMicroelectronics/meta-st-stm32mp
git clone -b kirkstone https://github.com/yoctoproject/poky
git clone -b kirkstone https://github.com/openembedded/meta-openembedded
git clone -b kirkstone https://github.com/agherzan/meta-raspberrypi


# Prepare environment
cp -a ~/Documents/my_yocto/meta-kernmod ~/projects/yocto/

git config --global user.email "pda761@mail.ru" && git config --global user.name "pet-r-off"

reboot
