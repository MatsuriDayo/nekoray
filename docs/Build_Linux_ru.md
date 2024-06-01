Компиляция Nekoray для Linux

## Клонируем репозиторий 

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## Простой метод компиляции

Зависимости：

1. Должны быть установлены зависимости C++ `protobuf yaml-cpp zxing-cpp` поддерживаемой версии
2. Должны быть установлены `qtbase` `qtsvg` `qttools` `qtx11extras`
3. Должен быть установлен Qt версии `5.12.x` или `5.15.x`
4. ОС системы `x86-64-linux-gnu`

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

После компиляции будет собран исполняемый файл `nekoray`

Распакуйте Release，замените `nekoray` на тот что скомпилировали, а `launcher` можно удалить

## Сложный метод компиляции

### Параметры CMake 

| Параметры CMake   | Значение по умолчанию| Текущее значение      |
|-------------------|----------------------|-----------------------|
| QT_VERSION_MAJOR  | 5                    | QT版本                 |
| NKR_NO_EXTERNAL   |                      | 不包含外部 C/C++ 依赖 (以下所有) |
| NKR_NO_YAML       |                      | 不包含 yaml-cpp        |
| NKR_NO_QHOTKEY    |                      | 不包含 qhotkey         |
| NKR_NO_ZXING      |                      | 不包含 zxing           |
| NKR_NO_GRPC       |                      | 不包含 gRPC            |
| NKR_NO_QUICKJS    |                      | 不包含 quickjs         |
| NKR_PACKAGE       |                      | 编译 package 版本 (aur)|
| NKR_PACKAGE_MACOS |                      | 编译 macos 版本        |
| NKR_LIBS          | ./libs/deps/built    | 依赖搜索目录            |
| NKR_DISABLE_LIBS  |                      | 禁用 NKR_LIBS          |

1. `NKR_LIBS` будет добавлен к `CMAKE_PREFIX_PATH`
2.  Если включена опция `NKR_PACKAGE`, то по умолчанию значение `NKR_LIBS` равно `. /libs/deps/package`，зависимости доступны в `build_deps_all.sh`
3.  Если включена опция`NKR_PACKAGE_MACOS` или `NKR_PACKAGE`，приложение будет использовать каталог appdata для хранения конфигурации, а автоматическое обновление будет отключено

### Зависимости C++

Если в дистрибутиве нет нужных зависимостей C++ или версия не поддерживается, можете запустить скрипт `build_deps_all.sh` для самостоятельной сборки зависимостей.

Требования: установлен Qt `5.12.x` или `5.15.x`.

#### Сборка и установка зависимостей C/C++

(этот шаг нужен для установки зависимостей)

```shell
./libs/build_deps_all.sh
```

#### Компиляция основного кода

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

Результатом компиляции будет исполняемый файл `nekoray`

### Компиляция Go кода 

Читайте [Build_Core_ru.md](./Build_Core_ru.md)
