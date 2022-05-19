#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

ip rule del table 114
ip rule del fwmark 514
ip route del table 114 default
ip -6 rule del table 114
ip -6 rule del fwmark 514
ip -6 route del table 114 default
ip link del matsuri-tun
