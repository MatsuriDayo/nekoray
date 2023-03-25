#!/bin/bash

version="$1"

mkdir -p nekoray/DEBIAN
mkdir -p nekoray/opt
mkdir -p nekoray/usr/local/bin
cp -r linux64 nekoray/opt/
mv nekoray/opt/linux64 nekoray/opt/nekoray

# basic
cat >nekoray/DEBIAN/control <<-EOF
Package: nekoray
Version: $version
Architecture: amd64
Maintainer: MatsuriDayo nekoha_matsuri@protonmail.com
Depends: libxcb-xinerama0, qtbase5-dev, libqt5svg5-dev, libqt5x11extras5-dev
Description: Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)
EOF

# Fix permission issues
cat >nekoray/DEBIAN/postinst <<-EOF
cat >/usr/share/applications/nekoray.desktop<<-END
[Desktop Entry]
Name=nekoray
Version=$version
Comment=Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)
Exec=/opt/nekoray/nekoray -appdata
Icon=/opt/nekoray/nekoray.png
Terminal=false
Type=Application
Categories=Network;Application;
END
chmod -R 777 /opt/nekoray/
chmod -R 777 /opt/nekoray/nekoray /opt/nekoray/nekoray_core /opt/nekoray/nekobox_core
chmod +x /opt/nekoray/nekoray /opt/nekoray/nekoray_core /opt/nekoray/nekobox_core
update-desktop-database
EOF

sudo chmod 755 nekoray/DEBIAN/postinst

# desktop && PATH

sudo dpkg-deb --build nekoray
