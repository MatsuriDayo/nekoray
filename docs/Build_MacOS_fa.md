Tip: Passing the compilation does not mean that all functions are available, and the function of Tun system proxy is probably not available. If you know how to fix it, we welcome your Pull Request.

## پروژه Nekoray را از منبع آن دریافت کنید
در `Terminal` در مسیر دلخواه خود، دستور زیر را وارد کنید و دکمه `return` را بفشارید.
```shell
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```
اگر `git` روی دستگاه از پیش نصب نباشد، دستگاه، خودکار نصب آن را پیشنهاد می‌کند. مرحله‌های نصب را به انجام برسانید و سپس دوباره دستور پیش گفته را در `Terminal` اجرا کنید.

سپس وارد پوشه `nekoray` شوید.
برای کامپایل کردن پروژه، پیش‌نیازهای زیر را نصب کنید. برای این، دستور زیر را در `Terminal` وارد کنید و دکمه `Return` را بفشارید.


```shell
cd nekoray
```
<br>

## پیش‌نیازها

###۱. `CMake`

برای نصب `CMake` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
brew install cmake
```
اگر با خطای `zsh: command not found: brew` روبه‌رو شدید به این معنی است که `homebrew` در دستگاه شما نصب نیست. برای نصب آن نخست باید `XCode's command line tools` را نصب کنید. اگر به این مشکل برخورده‌ اید، گام های `۱.۱`، `۱.۲` و `۱.۳` را دنبال کنید.

<br>

####۱.۱. نصب `XCode's command line tools`

دستور زیر را در `terminal` وادر کنید و دکمه `return` را بفشارید.
```shell
xcode-select --install
```

<br>

####۱.۲. نصب `homebrew`

دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
curl -fsSL -o install.sh https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh
```

<br>

####۱.۳. حال می‌توانید با دستوری که پیشتر گفتیم، `CMake` را نصب کنید.

<br>

###۲. `Ninja`

برای نصب `Ninja` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
sudo port install ninja
```
اگر با خطای `sudo: port: command not found` روبه‌رو شدید، به این معنی است که `MacPorts` روی دستگاه شما نصب نیست. نسخه `MacPorts` متناسب با نسخه `macOs` خود را بارگری و نصب کنید. سپس بار دیگر، دستور نصب `Ninja` را در `terminal` وارد کرده و دکمه `return` را بفشارید.

<br>

###۳. `npm`

برای نصب `npm` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
brew install npm
```

<br>

###۴. `qtbase`

برای نصب `qtbase` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
sudo port install qt5-qtbase
```

<br>

###۵. `qtsvg`

برای نصب `qtsvg` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
sudo port install qt5-qtsvg
```

<br>

###۶. `qttools`

برای نصب `qttools` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
sudo port install qt5-qttools
```

<br>

###۷. `appdmg`

برای نصب `appdmg` دستور زیر را در `terminal` وارد کنید و دکمه `return` را بفشارید.
```shell
sudo npm install -g appdmg
```

<br>

## C++

### وابستگی‌های C و ++C را کامپایل و نصب کنید.

دستور زیر را در `terminal` وارد کرده و دکمه `return` را بفشارید.

```shell
./libs/build_deps_all.sh
```

<br>

### آنتولوژی را کامپایل کنید.

دستور زیر را در `terminal` وارد کرده و دکمه `return` را بفشارید.
```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DNKR_PACKAGE_MACOS=1 ..
ninja
```

<br>

## هسته نرم افزار

### هسته نرم افزار را کامپایل کنید.

چگونگی کامپایل کردن هسته نرم افزار را [اینجا ](./Build_Core_fa.md)بخوانید.

<br>

### جایگذاری هسته

فایل کامپایل شده `nekoray_core` و `nekobox_core` را در مسیر زیر کپی کنید.
`build/nekoray.app/Contents/MacOS`

<br>

## فایل‌های جغرافی را تهیه کنید. (اختیاری)

اگر مایلید، با اجرای دستورهای زیر در `Terminal` فایل های جغرافی را تهیه کنید.

```shell
curl -Lso build/nekoray.app/Contents/MacOS/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geoip.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.dat "https://github.com/v2fly/domain-list-community/releases/latest/download/dlc.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geoip.db "https://github.com/SagerNet/sing-geoip/releases/latest/download/geoip.db"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.db "https://github.com/SagerNet/sing-geosite/releases/latest/download/geosite.db"
```
<br>

## پایان کار

فایل اجرایی `Nekoray` در مسیر زیر قرار دارد. می‌توانید آن را در مسیر دلخواه کپی کنید.

`مسیر پروژه Nekoray/build`