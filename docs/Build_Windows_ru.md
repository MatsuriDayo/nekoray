在 Windows 下编译 Nekoray

### git clone 源码

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

### 安装 Visual Studio

从微软官网安装，可以使用 2019 和 2022 版本，安装 Win32 C++ 开发环境。

安装好后可以在「开始」菜单找到 `x64 Native Tools Command Prompt`

本文之后的命令均在该 cmd 内执行。`cmake` `ninja` 等工具使用 VS 自带的即可。

### 下载 Qt SDK

目前 Windows Release 使用的版本是 Qt 6.5.x

下载解压后，将 bin 目录添加到环境变量。

#### Release 编译用到的 Qt 包下载 (MSVC2019 x86_64)

https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/Qt6.5.0-Windows-x86_64-VS2022-17.5.5-20230507.7z

#### 官方签名版 Qt 5.15.2 （可选，已知有内存泄漏的BUG）

在此下载 `qtbase` `qtsvg` `qttools` 的包并解压到同一个目录。

https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5152/qt.qt5.5152.win64_msvc2019_64/

### C++ 部分编译

#### 编译安装 C/C++ 依赖

（这一步可能要挂梯）

```shell
bash ./libs/build_deps_all.sh
```

目前只有 bash 脚本，没有批处理或 powershell，如果 Windows 没有带 bash 建议自行安装。

CMake 参数等细节与 Linux 大同小异，有问题可以参照 Build_Linux 文档。

#### 编译本体

请根据你的 QT Sdk 的位置替换命令

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/path/to/qt/5.15.2/msvc2019_64 ..
ninja
```

编译完成后得到 `nekoray.exe`

最后运行 `windeployqt nekoray.exe` 自动复制所需 DLL 等文件到当前目录

### Go 部分编译

请看 [Build_Core.md](./Build_Core.md)
