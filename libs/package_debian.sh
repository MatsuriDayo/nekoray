#!/bin/bash

version="$1"

mkdir -p nekoray/DEBIAN
mkdir -p nekoray/opt
cp -r linux64 nekoray/opt/
mv nekoray/opt/linux64 nekoray/opt/nekoray
rm -rf nekoray/opt/nekoray/usr
rm nekoray/opt/nekoray/launcher

# basic
cat >nekoray/DEBIAN/control <<-EOF
Package: nekoray
Version: $version
Architecture: amd64
Maintainer: MatsuriDayo nekoha_matsuri@protonmail.com
Depends: libxcb-xinerama0, libqt5core5a, libqt5gui5, libqt5network5, libqt5widgets5, libqt5svg5, libqt5x11extras5, desktop-file-utils
Description: Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)
EOF

cat >nekoray/DEBIAN/postinst <<-EOF
if [ ! -s /usr/share/applications/nekoray.desktop ]; then
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
else
    sed -i "s/^Version=.*/Version=$version/" /usr/share/applications/nekoray.desktop
fi

update-desktop-database
EOF

sudo chmod 755 nekoray/DEBIAN/postinst

# desktop && PATH

sudo dpkg-deb -Zxz --build nekoray
