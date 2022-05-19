#!/bin/sh
set -e
set -x

BASEDIR=$(dirname "$0")

PORT=$1
[[ -z $1 ]] && PORT="2080"

pkexec env PORT=$PORT sh -c "cd $PWD && $BASEDIR/vpn-run-root.sh"
