#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]; then
  echo "[Warning] Tun script not running as root"
fi

command -v pkill >/dev/null 2>&1 || echo "[Warning] pkill not found"

BASEDIR=$(dirname "$0")
cd $BASEDIR

pre_start_linux() {
  # for Tun2Socket
  iptables -I INPUT -s 172.19.0.2 -d 172.19.0.1 -p tcp -j ACCEPT
  ip6tables -I INPUT -s fdfe:dcba:9876::2 -d fdfe:dcba:9876::1 -p tcp -j ACCEPT
}

start() {
  pre_start_linux
  "./nekobox_core" run -c "$CONFIG_PATH"
}

stop() {
  iptables -D INPUT -s 172.19.0.2 -d 172.19.0.1 -p tcp -j ACCEPT
  ip6tables -D INPUT -s fdfe:dcba:9876::2 -d fdfe:dcba:9876::1 -p tcp -j ACCEPT
}

if [ "$1" != "stop" ]; then
  start || true
fi

stop || true
