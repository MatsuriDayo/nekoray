# NekoRay / NekoBox для ПК

Кросс-платформенный GUI менеджер на библиотеке Qt для конфигурации прокси (на базе ядер: v2ray / sing-box)

Поддерживает Windows / Linux из коробки.

## Загрузить

### GitHub Релизы (Портативный ZIP-архив)

Портативный формат, без установщика. Перейдите в раздел Releases, чтобы загрузить готовый исполняемый файл.

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[Загрузить](https://github.com/Matsuridayo/nekoray/releases)

[Инструкция по установке пакета, если вы не знаете, какой из них загрузить (на китайском)](https://github.com/MatsuriDayo/nekoray/wiki/Installation-package-description)

### Пакеты

#### AUR

- [nekoray](https://aur.archlinux.org/packages/nekoray)
- [nekoray-git](https://aur.archlinux.org/packages/nekoray-git) [![AUR CI](https://github.com/MatsuriDayo/nekoray/actions/workflows/update-pkgbuild.yml/badge.svg)](https://github.com/MatsuriDayo/nekoray/actions/workflows/update-pkgbuild.yaml)

#### archlinuxcn

- [nekoray](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/nekoray)
- [nekoray-git](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/nekoray-git)

#### Scoop Extras

`scoop install nekoray`

## Список изменений и Телеграм Канал

https://t.me/Matsuridayo

## Домашняя страница & Документация

https://matsuridayo.github.io

## Прокси

- SOCKS (4/4a/5)
- HTTP(S)
- Shadowsocks
- VMess
- VLESS
- Trojan
- TUIC ( sing-box )
- NaïveProxy ( Пользовательское ядро )
- Hysteria ( Пользовательское ядро или sing-box )
- Hysteria2 ( Пользовательское ядро или sing-box )
- Пользовательские Исходящие Запросы
- Пользовательская Конфигурация
- Пользовательское ядро

## Подписка

- Raw: некоторые широкоиспользуемые форматы (например, Shadowsocks, Clash и v2rayN)

## Параметры работы

[Параметры работы](docs/RunFlags.md)

## Запуск на Windows

Если приложение не запускается или появляется ошибка об отсутствии DLL, то загрузите и установите [Библиотеки среды выполнения Microsoft C++](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Запуск на Linux

[Гайд по запуску на Linux (на китайском)](docs/Run_Linux.md)

## macOS

Версия для macOS больше не поддерживается и недоступна для загрузки.

Вы можете скачать неофициальные обновленные версии в репозитории ниже. [Неофициальные релизы](https://github.com/aaaamirabbas/nekoray-macos/releases)

Чтобы скомпилировать самостоятельно, прочитайте руководство по компиляции ниже.
Пожалуйста, присмотритесь устаревшим релизам, чтобы узнать о распространенных проблемах. В случае обнаружения ошибок можете создать Pull Requests с исправлениями.

## Гайд по компиляции

Можно прочитать в [технической документации](https://github.com/MatsuriDayo/nekoray/tree/main/docs)

## Пожертвование

Если проект оказался полезным, его разработку можно поддержать, сделав пожертвование.

Пользователи которые пожертовали более 50 долларов США могут разместить свою аватарку на [Странице поддержавших](https://mtrdnt.pages.dev/donation_list). Если Вас не добавили, не стесняйтесь написать нам.

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`
  
## Авторы

Ядра:

- [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core) ( < 3.10 )
- [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri) ( < 3.10 )
- [MatsuriDayo/v2ray-core](https://github.com/MatsuriDayo/v2ray-core) ( < 3.10 )
- [XTLS/Xray-core](https://github.com/XTLS/Xray-core) ( >= 3.10 )
- [MatsuriDayo/Xray-core](https://github.com/MatsuriDayo/Xray-core) ( >= 3.10 )
- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [Matsuridayo/sing-box-extra](https://github.com/MatsuriDayo/sing-box-extra)

Gui:

- [Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [Qt](https://www.qt.io/)
- [protobuf](https://github.com/protocolbuffers/protobuf)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [zxing-cpp](https://github.com/nu-book/zxing-cpp)
- [QHotkey](https://github.com/Skycoder42/QHotkey)
- [AppImageKit](https://github.com/AppImage/AppImageKit)
