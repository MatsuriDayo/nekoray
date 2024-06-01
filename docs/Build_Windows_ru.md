Компиляция Nekoray для Windows

### Клонируем репозиторий

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

### Установите Visual Studio

На сайте Microsoft вы можете установить среду разработки Win32 C++ версий 2019 и 2022.

После установки вы можете найти командную строку `x64 Native Tools Command Prompt` в меню Пуск.

Команды, приведенные в этой статье, выполняются в cmd. С VS можно использовать такие инструменты, как `cmake` и `ninja`.

### Установите Qt SDK

Текущая версия Qt для Windows Release - Qt 6.5.x.

После загрузки и извлечения SDK добавьте каталог bin в переменную окружения.

#### Скачайте пакеты Qt для компиляции сборки (MSVC2019 x86_64)

https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/Qt6.5.0-Windows-x86_64-VS2022-17.5.5-20230507.7z

#### Официальная подписанная версия Qt 5.15.2 (необязательно, на данный момент в этой версии есть ошибки утечки памяти).

Скачайте пакеты `qtbase` `qtsvg` `qttools` здесь и распакуйте их в тот же каталог.

https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5152/qt.qt5.5152.win64_msvc2019_64/

## C++ окружение

#### Сборка и установка зависимостей C/C++

(этот шаг нужен для установки зависимостей)

```shell
./libs/build_deps_all.sh
```

На данный момент существуют только скрипты для bash, без batch и powershell, поэтому если на Windows не установлен bash, мы рекомендуем установить его самостоятельно.

Параметры CMake и другие детали во многом совпадают с Linux, если у вас возникнут вопросы, обратитесь к документации [Build_Linux_ru.md](./Build_Linux_ru.md).

#### Компиляция основного кода

Замените команды в соответствии с расположением вашего QT Sdk.

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/path/to/qt/5.15.2/msvc2019_64 ..
ninja
```

После компиляции вы получите `nekoray.exe`.

Наконец, запустите `windeployqt nekoray.exe` для автоматического копирования необходимых DLL и других файлов в текущий каталог.

### Компиляция GO кода

Смотрите [Build_Core_ru.md](./Build_Core_ru.md)
