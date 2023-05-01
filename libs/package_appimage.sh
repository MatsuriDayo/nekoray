#!/bin/bash

sudo apt-get install fuse -y

cp -r linux64 nekoray.AppDir

# The file for Appimage

cat >nekoray.AppDir/nekoray.desktop<<-EOF
[Desktop Entry]
Name=nekoray
Exec=/launcher -- -appdata
Icon=nekoray
Type=Application
Categories=Network
EOF

cat >nekoray.AppDir/AppRun<<-EOF
#!/bin/bash
HERE="\$(dirname "\$(readlink -f "\${0}")")"
\${HERE}/launcher -- -appdata
EOF

chmod +x nekoray.AppDir/AppRun

# build

curl -fLSO https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage nekoray.AppDir

# remove

rm appimagetool-x86_64.AppImage
rm -rf nekoray.AppDir