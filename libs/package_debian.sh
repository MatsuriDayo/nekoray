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

# Start VPN mode without password
cat >nekoray/opt/nekoray/pkexec <<-EOF
#!/bin/sh

if [ \$1 = --help ]; then
  echo "This is not real pkexec."
  exit 0
fi

TO_EXEC="\$@"

if [ \$1 = --keep-cwd ]; then
  TO_EXEC="\${@:2}"
fi

\$TO_EXEC
EOF

cat >nekoray/DEBIAN/postinst <<-EOF
if [ ! -s /usr/share/applications/nekoray.desktop ]; then
    cat >/usr/share/applications/nekoray.desktop<<-END
[Desktop Entry]
Name=nekoray
Comment=Qt based cross-platform GUI proxy configuration manager (backend: Xray / sing-box)
Exec=sh -c "PATH=/opt/nekoray:\$PATH /opt/nekoray/nekoray -flag_linux_run_core_as_admin -appdata"
Icon=/opt/nekoray/nekoray.png
Terminal=false
Type=Application
Categories=Network;Application;
END
fi

setcap cap_net_admin=ep /opt/nekoray/nekobox_core
chmod +x /opt/nekoray/pkexec
chmod 0755 /opt/nekoray/pkexec

update-desktop-database
EOF

sudo chmod 0755 nekoray/DEBIAN/postinst

# desktop && PATH

sudo dpkg-deb -Zxz --build nekoray
