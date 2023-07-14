#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]; then
  echo "[Warning] Tun script not running as root"
fi

if [ "$(uname)" == "Darwin" ]; then
  IS_MACOS=1
fi

[ -z $TABLE_FWMARK ] && echo "Please set env TABLE_FWMARK" && exit
command -v pkill >/dev/null 2>&1 || echo "[Warning] pkill not found"

BASEDIR=$(dirname "$0")
cd $BASEDIR

pre_start_linux() {
  # set bypass: fwmark
  ip rule add pref 8999 fwmark $TABLE_FWMARK table main || return
  ip -6 rule add pref 8999 fwmark $TABLE_FWMARK table main || return

  # for Tun2Socket
  iptables -I INPUT -s 172.19.0.2 -d 172.19.0.1 -p tcp -j ACCEPT
  ip6tables -I INPUT -s fdfe:dcba:9876::2 -d fdfe:dcba:9876::1 -p tcp -j ACCEPT
}

start() {
  [ -z $IS_MACOS ] && pre_start_linux
  "./nekobox_core" run -c "$CONFIG_PATH" --protect-listen-path "$PROTECT_LISTEN_PATH" --protect-fwmark $TABLE_FWMARK
}

stop() {
  [ -z $IS_MACOS ] || return
  for local in $BYPASS_IPS; do
    ip rule del to $local table main
  done
  iptables -D INPUT -s 172.19.0.2 -d 172.19.0.1 -p tcp -j ACCEPT
  ip6tables -D INPUT -s fdfe:dcba:9876::2 -d fdfe:dcba:9876::1 -p tcp -j ACCEPT
  ip rule del fwmark $TABLE_FWMARK
  ip -6 rule del fwmark $TABLE_FWMARK
}

if [ "$1" != "stop" ]; then
  start || true
fi

stop || true
