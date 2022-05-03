#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
fi

# add netns
ip netns add nekoray
# ip netns exec nekoray readlink /proc/self/ns/net

# add lo: lo is not shared
ip -n nekoray addr add 127.0.0.1/8 dev lo
ip -n nekoray link set dev lo up

# add tun
ip -n nekoray tuntap add tun0 user $USERID mode tun
ip -n nekoray addr add 26.0.0.1/30 dev tun0
ip -n nekoray link set dev tun0 up
ip -n nekoray route add default dev tun0

# set veth to use the socks port
ip link add dev nekoray-ve1 type veth peer name nekoray-ve2
ip addr add 26.1.0.1/30 dev nekoray-ve1
ip link set nekoray-ve1 up
ip link set nekoray-ve2 netns nekoray
ip -n nekoray addr add 26.1.0.2/30 dev nekoray-ve2
ip -n nekoray link set nekoray-ve2 up
