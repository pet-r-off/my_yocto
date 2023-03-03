#!/bin/bash
sudo swapon -s
sudo swapoff -v /swapfile
sudo swapon -s
sudo rm -rf /swapfile
sudo fallocate -l 32G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile