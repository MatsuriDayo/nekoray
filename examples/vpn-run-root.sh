#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
fi

[ -z $PORT ] && echo "Please set env PORT" && exit
[ -z $TABLE_FWMARK ] && echo "Please set env TABLE_FWMARK" && exit
[ -z $TUN_NAME ] && echo "Please set env TUN_NAME" && exit
[ -z $USER_ID ] && echo "Please set env USER_ID" && exit
[ -z $MTU ] && MTU=1500
command -v pkill >/dev/null 2>&1 || exit

BASEDIR=$(dirname "$0")
cd $BASEDIR

start() {
  # add tun
  ip tuntap add $TUN_NAME mode tun user $USER_ID || return
  ip link set dev nekoray-tun mtu $MTU || return
  ip link set dev $TUN_NAME up || return

  # set ipv4 rule
  ip addr add 172.19.0.1/30 dev $TUN_NAME || return
  ip rule add table $TABLE_FWMARK || return
  ip route add table $TABLE_FWMARK default dev $TUN_NAME || return

  # set ipv6 rule
  ip -6 rule add table $TABLE_FWMARK || return
  if [ -z $ENABLE_IPV6 ]; then
    ip -6 route add table $TABLE_FWMARK unreachable default || return
  else
    ip -6 addr add fdfe:dcba:9876::1/126 dev $TUN_NAME || return
    ip -6 route add table $TABLE_FWMARK default dev $TUN_NAME || return
  fi

  # set bypass: fwmark
  ip rule add fwmark $TABLE_FWMARK table main || return
  ip -6 rule add fwmark $TABLE_FWMARK table main || return

  # set bypass: LAN
  for local in $BYPASS_IPS; do
    ip rule add to $local table main
  done

  # for Tun2Socket
  iptables -I INPUT -s 172.19.0.2 -d 172.19.0.1 -p tcp -j ACCEPT
  ip6tables -I INPUT -s fdfe:dcba:9876::2 -d fdfe:dcba:9876::1 -p tcp -j ACCEPT

  if [ ! -z $USE_NEKORAY ]; then
    "./nekoray_core" tool protect --protect-listen-path "$PROTECT_LISTEN_PATH" --protect-fwmark $TABLE_FWMARK
  else
    if [ -z "$PROTECT_LISTEN_PATH" ]; then
      "./tun2socks" -device $TUN_NAME -proxy socks5://127.0.0.1:$PORT -interface lo
    else
      "./tun2socks" -device $TUN_NAME -proxy socks5://127.0.0.1:$PORT -interface lo --protect-listen-path "$PROTECT_LISTEN_PATH" --protect-fwmark $TABLE_FWMARK
      rm "$PROTECT_LISTEN_PATH"
    fi
  fi
}

stop() {
  for local in $BYPASS_IPS; do
    ip rule del to $local table main
  done
  iptables -D INPUT -s 172.19.0.2 -d 172.19.0.1 -p tcp -j ACCEPT
  ip6tables -D INPUT -s fdfe:dcba:9876::2 -d fdfe:dcba:9876::1 -p tcp -j ACCEPT
  ip rule del table $TABLE_FWMARK
  ip rule del fwmark $TABLE_FWMARK
  ip route del table $TABLE_FWMARK default
  ip -6 rule del table $TABLE_FWMARK
  ip -6 rule del fwmark $TABLE_FWMARK
  ip -6 route del table $TABLE_FWMARK default
  ip link del $TUN_NAME
}

if [ "$1" != "stop" ]; then
  start || true
fi

stop || true
