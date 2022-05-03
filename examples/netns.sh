#!/bin/sh
set -e
set -x

BASEDIR=$(dirname "$0")

# netns
[ -f /var/run/netns/nekoray ] || pkexec env USERID=`id -u` sh -c "cd $PWD && $BASEDIR/netns-root.sh" || true

# run xjasonlyu/tun2socks to provide vpn
firejail --noprofile --netns=nekoray ./tun2socks -device tun0 -proxy socks5://26.1.0.1:2080 -interface nekoray-ve2 -drop-multicast

# use "firejail --noprofile --netns=nekoray ..." to run your program in VPN
