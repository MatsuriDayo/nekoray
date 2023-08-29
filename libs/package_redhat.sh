#!/bin/bash

VERSION="$1"

alien -r -c nekoray*.deb

mv "nekoray*.rpm" "${VERSION}-redhat-x64.rpm"