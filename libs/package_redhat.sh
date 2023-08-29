#!/bin/bash

VERSION="$1"

sudo alien -r -c nekoray*.deb

sudo mv "*.rpm" "${VERSION}-redhat-x64.rpm"