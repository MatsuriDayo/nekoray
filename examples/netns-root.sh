#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# add netns
ip netns add matsuri
# ip netns exec matsuri readlink /proc/self/ns/net

# add lo: lo is not shared
ip -n matsuri addr add 127.0.0.1/8 dev lo
ip -n matsuri link set dev lo up

# add tun
ip -n matsuri tuntap add tun0 user $USERID mode tun
ip -n matsuri addr add 26.0.0.1/30 dev tun0
ip -n matsuri link set dev tun0 up
ip -n matsuri route add default dev tun0

# set veth to use the socks port
ip link add dev matsuri-ve1 type veth peer name matsuri-ve2
ip addr add 26.1.0.1/30 dev matsuri-ve1
ip link set matsuri-ve1 up
ip link set matsuri-ve2 netns matsuri
ip -n matsuri addr add 26.1.0.2/30 dev matsuri-ve2
ip -n matsuri link set matsuri-ve2 up
