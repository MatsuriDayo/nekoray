Compile Nekoray under Windows

### git clone source code

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

### Install Visual Studio

Installed from Microsoft's official website, you can use the 2019 and 2022 versions to install the Win32 C++ development environment.

After installation, you can find `x64 Native Tools Command Prompt` in the "Start" menu

The commands after this article are all executed in this cmd. For tools such as `cmake` `ninja`, you can use the tools that come with VS.

### Download Qt SDK

The currently used version is Qt 5.15.x

Qt 6.x is temporarily deprecated

After downloading and decompressing, add the bin directory to the environment variable.

#### Qt 5.15.7 MSVC2019 x86_64

<https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/Qt5.15.7-Windows-x86_64-VS2019-16.11.20-20221103.7z>

#### Official signed version Qt 5.15.2 (optional, known to have memory leak bugs)

Download `qtbase` `qtsvg` `qttools` packages here and unzip them to the same directory.

<https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5152/qt.qt5.5152.win64_msvc2019_64/>

### C++ Partial Compilation

#### Compile and install C/C++ dependencies

(This step may require a ladder)

```shell
bash ./libs/build_deps_all.sh
```

At present, there are only bash scripts, no batch processing or powershell. If Windows does not have bash, it is recommended to install it by yourself.

CMake parameters and other details are similar to Linux, if you have any questions, please refer to the Build_Linux document.

#### Compile Ontology

Please replace the command according to the location of your QT Sdk

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/path/to/qt/5.15.2/msvc2019_64 ..
ninja
```

After compiling, you get `nekoray.exe`

Finally, run `windeployqt nekoray.exe` to automatically copy the required DLL and other files to the current directory

### Go partial compilation

with Linux
