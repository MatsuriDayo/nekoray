## Linux Installation

For installation, you can use this script:  
```sh
sh install-nekoray.sh
```

### Debian-Based Distributions

For Debian-based distributions, it is recommended to use the `.deb` package for installation:

```shell
sudo apt install ./nekoray-*-debian-x64.deb
```

After installation, the desktop shortcut will launch with the `-appdata` parameter. If you want to launch directly and use previous configurations, make sure to include this parameter.

### Arch-Based Distributions

For Arch-based distributions, it is recommended to install from `AUR` or `archlinuxcn`:

#### AUR
##### Latest Stable Version

```shell
[yay/paru] -S nekoray
```

##### Latest Git Version (Development Version)

```shell
[yay/paru] -S nekoray-git
```

#### archlinuxcn

##### Latest Stable Version

```shell
sudo pacman -S nekoray
```

##### Latest Git Version (Development Version)

```shell
sudo pacman -S nekoray-git
```

### Other Distributions

Download the `.zip` file, extract it to a suitable directory, and use it right away.

Alternatively, download the `.AppImage` file and grant execution permissions using:

```shell
chmod +x nekoray-*-AppImage-x64.AppImage
```

Refer to the following sections for detailed usage instructions.

## Linux Usage

**Using a Linux system implies that you have basic troubleshooting skills.  
This project does not provide specific support for distributions/architectures. If the precompiled files do not meet your needs, please compile/adjust them yourself.**

Note: Some Linux distributions are known to be incompatible, and there is no current support for non-x86_64 architectures. You may attempt to compile the software yourself.

For the current release, portable packages can be used in two ways after extraction:

1. **System Mode**: To use the system's Qt5 runtime library, execute `./nekoray`.
2. **Bundle Mode**: To use the precompiled Qt runtime library, execute `./launcher`.

### Bundle Mode

**Requirements**: A mainstream distribution with an xcb desktop environment installed.

**Run**: Execute `./launcher`, or double-click to open on some systems.

Launcher parameters:

- `./launcher -- -appdata` (Parameters after `--` are passed to the main program.)
- `-debug` Debug mode

For Ubuntu 22.04:  
```shell
sudo apt install libxcb-xinerama0
```

### System Mode

**Requirements**: A mainstream distribution with an xcb desktop environment installed and a Qt5.12 ~ Qt5.15 environment set up.

**Run**: Execute `./nekoray`, or double-click to open on some systems. If it fails to run, it is recommended to use the Bundle version.

