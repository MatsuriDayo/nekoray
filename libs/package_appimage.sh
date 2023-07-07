#!/bin/bash

sudo apt-get install fuse -y

cp -r linux64 nekoray.AppDir

# The file for Appimage

rm nekoray.AppDir/launcher

cat >nekoray.AppDir/nekoray.desktop <<-EOF
[Desktop Entry]
Name=nekoray
Exec=echo "NekoRay started"
Icon=nekoray
Type=Application
Categories=Network
EOF

cat >nekoray.AppDir/AppRun <<-EOF
#!/bin/bash
echo "PATH: \${PATH}"
echo "NekoRay runing on: \$APPDIR"
LD_LIBRARY_PATH=\${APPDIR}/usr/lib QT_PLUGIN_PATH=\${APPDIR}/usr/plugins \${APPDIR}/nekoray -appdata "\$@"
EOF

chmod +x nekoray.AppDir/AppRun

# build

curl -fLSO https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage nekoray.AppDir

# clean

rm appimagetool-x86_64.AppImage
rm -rf nekoray.AppDir
