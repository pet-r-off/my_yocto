#!/bin/bash


# Download yocto build dependencies 
if [ "$1" == "--install" ];
then
	# Update
	sudo apt update
	sudo apt upgrade

	# Download dependencies
	echo "Installing Yocto build dependencies..."
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
fi


# Download libsfml-dev build dependencies
if [ "$1" == "--sfml" ] || [ "$2" == "--sfml" ];
then
	# Update
	sudo apt update
	sudo apt upgrade

	# Download dependencies
	echo "Installing SFML build dependencies..."
	sudo apt-get install -y freetype* libx11-dev libxrandr-dev x11-xserver-utils \ 
			udev libudev-dev libflac-dev libfreetype-dev mesa-utils flac libogg-dev \
			libvorbis-dev vorbis-tools libvorbisenc2 libvorbisfile3 \
			libalut-dev libpthread-stubs0-dev
fi


echo "alias python=python3" >> ~/.bashrc
python --version
source ~/.bashrc
python --version


sudo snap install code --classic
code --install-extension microhobby.bitbake2
code --install-extension ms-python.python
code --install-extension ms-python.vscode-pylance
code --install-extension ms-vscode.cpptools
code --install-extension NicScott.dark-bash
code --install-extension PKief.material-icon-theme
code --install-extension plorefice.devicetree
code --install-extension adpyke.codesnap
code --install-extension bmalehorn.shell-syntax
code --install-extension castello-dev.bash-snippets
code --install-extension designbyajay.bash-cli-snippets
code --install-extension ms-vscode.makefile-tools
code --install-extension mhutchie.git-graph
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake


# Clone git repos
if [ "$2" == "--yoctorepo" ] || [ "$3" == "--yoctorepo" ];
then
	echo "Cloning Yocto repos..."
	mkdir -p ~/projects/yocto/
	cd ~/projects/yocto/
	git clone -b kirkstone https://github.com/STMicroelectronics/meta-st-stm32mp
	git clone -b kirkstone https://github.com/STMicroelectronics/meta-st-openstlinux
	git clone -b kirkstone https://github.com/yoctoproject/poky
	git clone -b kirkstone https://github.com/openembedded/meta-openembedded
	git clone -b kirkstone https://github.com/agherzan/meta-raspberrypi
	git clone -b kirkstone https://github.com/meta-qt5/meta-qt5
fi


# Prepare environment
git config --global user.email "pda761@mail.ru" && git config --global user.name "pet-r-off"

reboot
