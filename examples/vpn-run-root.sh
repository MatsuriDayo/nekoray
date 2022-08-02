#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
fi

command -v pkill >/dev/null 2>&1 || exit

BASEDIR=$(dirname "$0")
cd $BASEDIR

if [ "$1" != "stop" ]; then
  # add tun
  ip tuntap add nekoray-tun mode tun
  ip addr add 26.0.0.1/30 dev nekoray-tun
  ip link set dev nekoray-tun up

  # set ipv4 rule
  ip rule add table 114
  ip route add table 114 default dev nekoray-tun

  # set ipv6 unreachable
  ip -6 rule add table 114
  ip -6 route add table 114 unreachable default

  # set bypass: fwmark 514 (hardcoded)
  ip rule add fwmark 514 table main
  ip -6 rule add fwmark 514 table main

  # run xjasonlyu/tun2socks to provide vpn
  if [ -z $PROTECT_LISTEN_PATH ]; then
    ./tun2socks -device nekoray-tun -proxy socks5://127.0.0.1:$PORT -interface lo || true
  else
    ./tun2socks -device nekoray-tun -proxy socks5://127.0.0.1:$PORT -interface lo --protect-listen-path $PROTECT_LISTEN_PATH --protect-fwmark $PROTECT_FWMARK || true
    rm $PROTECT_LISTEN_PATH || true
  fi
fi

# stop
ip rule del table 114 || true
ip rule del fwmark 514 || true
ip route del table 114 default || true
ip -6 rule del table 114 || true
ip -6 rule del fwmark 514 || true
ip -6 route del table 114 default || true
ip link del nekoray-tun || true
