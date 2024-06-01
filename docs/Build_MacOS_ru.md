Компиляция не означает, что все функции будут доступны, например Tun System Agent скорее всего работать не будет. Если вы знаете, как это исправить, мы будем рады вашему Pull Request.

## Клонируем репозиторий

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## Зависимости

1. Должен быть установлен `CMake` `Ninja` `npm`
2. Должен быть установлен Qt `5.15.x` и задайте соответствующие переменные окружения, чтобы CMake не находил Qt. (`qtbase` `qtsvg` `qttools`)
3. Должен быть установлен `appdmg` (`sudo npm install -g appdmg`)

## Параметры CMake

Те же самые как для [Build_Linux_ru.md](./Build_Linux_ru.md)

## C++ окружение

#### Сборка и установка зависимостей C/C++

(этот шаг нужен для установки зависимостей)

```shell
./libs/build_deps_all.sh
```

### Компиляция основного кода

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DNKR_PACKAGE_MACOS=1 ..
ninja
```

### Компиляция GO кода

Смотрите [Build_Core_ru.md](./Build_Core_ru.md)

## Сборка

Если есть сомнения, смотрите [этот скрипт](https://github.com/MatsuriDayo/nekoray/blob/main/libs/deploy_macos.sh)

### Подготовьте ядро

Поместите скомпилированное `nekoray_core` `nekobox_core` в `build/nekoray.app/Contents/MacOS`

### Подготовка geo файлов (необязательно)

```shell
curl -Lso build/nekoray.app/Contents/MacOS/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geoip.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.dat "https://github.com/v2fly/domain-list-community/releases/latest/download/dlc.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geoip.db "https://github.com/SagerNet/sing-geoip/releases/latest/download/geoip.db"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.db "https://github.com/SagerNet/sing-geosite/releases/latest/download/geosite.db"
```

Если файл geo не включен в пакет, установите `v2ray resource path` внутри приложения

### Упаковка в каталог nekoray

```shell
appdmg appdmg.json /path/to/nekoray.dmg
```
