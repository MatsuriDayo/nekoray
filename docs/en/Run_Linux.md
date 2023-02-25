## Linux running

**Using the Linux system, I believe you have the basic troubleshooting ability,
This project does not provide support for specific releases/architectures. If the precompiled files cannot meet your needs, please compile/adapt them yourself.**

It is known that some Linux distributions cannot be used, and there is no adaptation for non-x86_64. You can try to compile it yourself.

At present, after the Release portable package is decompressed, there are two ways to use it:

1. System: To use the system's Qt5 runtime library, please execute `./nekoray`
2. Bundle: To use the precompiled Qt runtime library, please execute `./launcher`

### Bundle

Requirements: A major distribution and the xcb desktop environment have been installed.

Run: `./launcher` or some systems can double-click to open

launcher parameter

* `./launcher -- -appdata` (parameters after `--` are passed to the main program)
* `-debug` Debug mode

Ubuntu 22.04: `sudo apt install libxcb-xinerama0`

### System

Requirements: The mainstream release version and xcb desktop environment have been installed, and the Qt5.12 ~ Qt5.15 environment has been installed.

Run: `./nekoray` or some systems can double-click to open. If it doesn't work, it is recommended to use the Bundle version.
