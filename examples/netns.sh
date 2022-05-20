#!/bin/sh
set -e
set -x

BASEDIR=$(dirname "$0")

# netns
[ -f /var/run/netns/matsuri ] || pkexec env USERID=`id -u` sh -c "cd $PWD && $BASEDIR/netns-root.sh" || true

# run xjasonlyu/tun2socks to provide vpn
firejail --noprofile --netns=matsuri ./tun2socks -device tun0 -proxy socks5://26.1.0.1:2080 -interface matsuri-ve2 -drop-multicast

# use "firejail --noprofile --netns=matsuri ..." to run your program in VPN
