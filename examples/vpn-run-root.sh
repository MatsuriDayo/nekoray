#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

BASEDIR=$(dirname "$0")

# ## setup cap core
# sh $BASEDIR/set-cap.sh

# add tun
ip tuntap add matsuri-tun mode tun
ip addr add 26.0.0.1/30 dev matsuri-tun
ip link set dev matsuri-tun up

# set ipv4 rule
ip rule add table 114
ip route add table 114 default dev matsuri-tun

# set ipv6 unreachable
ip -6 rule add table 114
ip -6 route add table 114 unreachable default

# set bypass: fwmark 514 (hardcoded)
ip rule add fwmark 514 table main
ip -6 rule add fwmark 514 table main

# run xjasonlyu/tun2socks to provide vpn
./tun2socks -device matsuri-tun -proxy socks5://127.0.0.1:$PORT -interface lo || true

# stop
sh $BASEDIR/vpn-stop.sh
