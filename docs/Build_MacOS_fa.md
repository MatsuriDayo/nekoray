# کامپایل کردن Nekoray در macOs

## پروژه Nekoray را از منبع آن دریافت کنید
در terminal در مسیر دلخواه خود، دستور زیر را وارد کنید و دکمه return را بفشارید.
```shell
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## پیش‌نیازها
برای کامپایل کردن پروژه، پیش‌نیازهای زیر را نصب کنید.

۱. `CMake`
برای نصب CMake دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
brew install cmake
```
اگر با خطای `zsh: command not found: brew` روبه‌رو شدید به این معنی است که `homebrew` در دستگاه شما نصب نیست. برای نصب آن نخست باید XCode's command line tools را نصب کنید. اگر به این مشکل برخورده‌اید، گام‌های ۱.۱، ۱.۲ و ۱.۳ را دنبال کنید.
۱.۱. نصب XCode's command line tools
دستور زیر را در terminal وادر کنید و دکمه return را بفشارید.
```shell
xcode-select --install
```
۱.۲. نصب homebrew
دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
curl -fsSL -o install.sh https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh
```
۱.۳. حال می‌توانید با دستوری که پیشتر گفتیم، CMake را نصب کنید.

۲. `Ninja`
برای نصب Ninja دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
sudo port install ninja
```
اگر با خطای `sudo: port: command not found` روبه‌رو شدید، به این معنی است که MacPorts روی دستگاه شما نصب نیست. نسخه MacPorts متناسب با نسخه macOs خود را بارگری و نصب کنید. سپس بار دیگر، دستور نصب Ninja را در terminal وارد کرده و دکمه return را بفشارید.

۳. `npm`
بدای نصب npm دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
brew install node
```

۴. `qtbase`
برای نصب qtbase دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
sudo port install qt5-qtbase
```

۵. `qtsvg`
برای نصب qtsvg دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
sudo port install qt5-qtsvg
```

۶. `qttools`
برای نصب qttools دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
sudo port install qt5-qttools
```

۷. `appdmg`
برای نصب appdmg دستور زیر را در terminal وارد کنید و دکمه return را بفشارید.
```shell
sudo npm install -g appdmg
```

## C++

### وابستگی‌های C و ++C را کامپایل و نصب کنید.
دستور زیر را در terminal وارد کرده و دکمه return را بفشارید.

```shell
./libs/build_deps_all.sh
```

### آنتولوژی را کامپایل کنید.
دستور زیر را در terminal وارد کرده و دکمه return را بفشارید.
```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DNKR_PACKAGE_MACOS=1 ..
ninja
```

### هسته نرم‌افزار را کامپایل کنید.
چگونگی کامپایل کردن هسته نرم‌افزار را [اینجا ](./Build_Core.md)بخوانید.

### جایگذاری هسته
فایل کامپایل شده `nekoray_core` و `nekobox_core` را در مسیر زیر کپی کنید.
`build/nekoray.app/Contents/MacOS`


### فایل‌های جغرافی را تهیه کنید. (اختیاری)
اگر مایلید با اجرای دستورهای زیر در terminal فایل‌های جغدافی را تهیه کنید.

```shell
curl -Lso build/nekoray.app/Contents/MacOS/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geoip.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.dat "https://github.com/v2fly/domain-list-community/releases/latest/download/dlc.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geoip.db "https://github.com/SagerNet/sing-geoip/releases/latest/download/geoip.db"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.db "https://github.com/SagerNet/sing-geosite/releases/latest/download/geosite.db"
```

### پایان کار
فایل اجرایی Nekoray در مسیر زیر قرار دارد. می‌توانید آن را در مسیر دلخواه کپی کنید.
`مسیر پروژه Nekoray/build`