#!/bin/bash

sudo apt-get install fuse -y

cp -r linux64 nekobox.AppDir

# The file for Appimage

rm nekobox.AppDir/launcher

cat >nekobox.AppDir/nekobox.desktop <<-EOF
[Desktop Entry]
Name=nekobox
Exec=echo "nekobox started"
Icon=nekobox
Type=Application
Categories=Network
EOF

cat >nekobox.AppDir/AppRun <<-EOF
#!/bin/bash
echo "PATH: \${PATH}"
echo "nekobox runing on: \$APPDIR"
LD_LIBRARY_PATH=\${APPDIR}/usr/lib QT_PLUGIN_PATH=\${APPDIR}/usr/plugins \${APPDIR}/nekobox -appdata "\$@"
EOF

chmod +x nekobox.AppDir/AppRun

# build

curl -fLSO https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage nekobox.AppDir

# clean

rm appimagetool-x86_64.AppImage
rm -rf nekobox.AppDir
