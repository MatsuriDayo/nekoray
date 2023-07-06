#!/bin/bash

sudo apt-get install fuse -y

cp -r linux64 nekoray.AppDir

# The file for Appimage

# Start VPN mode without password
cat >nekoray.AppDir/pkexec <<-EOF
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

rm nekoray.AppDir/launcher

cat >nekoray.AppDir/nekoray.desktop <<-EOF
[Desktop Entry]
Name=nekoray
Exec=echo "NekoRay"
Icon=nekoray
Type=Application
Categories=Network
EOF

cat >nekoray.AppDir/AppRun <<-EOF
#!/bin/bash
HERE="\$(dirname "\$(readlink -f "\${0}")")"
export PATH=\${HERE}/:\$PATH 
echo "PATH: \${PATH}"
LD_LIBRARY_PATH=\${HERE}/usr/lib QT_PLUGIN_PATH=\${HERE}/usr/plugins \${HERE}/nekoray -appdata "\$@"
EOF

chmod +x nekoray.AppDir/AppRun
chmod +X nekoray.AppDir/pkexec

# build

curl -fLSO https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage nekoray.AppDir

# clean

rm appimagetool-x86_64.AppImage
rm -rf nekoray.AppDir
