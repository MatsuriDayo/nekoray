#!/bin/sh
set -e
set -x

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

killall nekoray_core || true
cp nekoray_core /opt/nekoray_core
cp geo* /opt/
setcap cap_net_admin+ep /opt/nekoray_core
ln -sf /opt/nekoray_core nekoray_core_cap
