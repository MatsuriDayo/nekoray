#!/bin/bash

VERSION="$1"

alien -r -c nekoray*.deb --to-rpm "$VERSION-redhat-x64.rpm"