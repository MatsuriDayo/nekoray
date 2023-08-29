#!/bin/bash

VERSION="$1"

sudo alien -r -c nekoray*.deb

ls

sudo mv "nekoray-*.rpm" "${VERSION}-redhat-x64.rpm"