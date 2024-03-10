# NekoRay / NekoBox для ПК

Кроссплатформенный пользовательский интерфейс (GUI) для управления конфигурациями прокси-серверов, основанный на фреймворке Qt и работающий на V2Ray и sing-box.

Поддерживает Windows / Linux из коробки.

## Переводы README

[中文 / English](README.md) / [fa_IR](README_fa.md) / [日本語](README_ja.md) / Русский

## Загрузить

### GitHub Релизы (портативный ZIP-архив)

Портативный формат без установщика. Перейдите в раздел Релизов (Releases) для загрузки предварительно скомпилированных готовых к использованию пакетов.

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

## Домашняя страница и документация

https://matsuridayo.github.io

## Прокси

- SOCKS (4/4a/5)
- HTTP(S)
- Shadowsocks
- VMess
- VLESS
- Trojan
- TUIC (sing-box)
- NaïveProxy (пользовательское ядро)
- Hysteria (пользовательское ядро или sing-box)
- Hysteria2 (пользовательское ядро или sing-box)
- Пользовательский исходящий трафик
- Пользовательская конфигурация
- Пользовательское ядро

## Подпись

- Raw: некоторые широко используемые форматы (например, Shadowsocks, Clash и v2rayN)

## Параметры работы

[Параметры работы (на китайском)](docs/RunFlags.md)

## Запуск на Windows

Если появляется ошибка об отсутствии DLL или приложение не запускается, загрузите и установите [Библиотеки среды выполнения Microsoft C++](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Запуск на Linux

[Гайд по запуску на Linux (на китайском)](docs/Run_Linux.md)

## macOS

Из-за отсутствия поддержки версия для macOS больше не доступна для загрузки.
Однако вы можете скачать неофициальные версии в репозитории ниже. [Неофициальные релизы](https://github.com/aaaamirabbas/nekoray-macos/releases)

Обратитесь к руководству ниже для самостоятельной компиляции пакетов. Пожалуйста, присмотритесь к устаревшим релизам, чтобы узнать о распространённых проблемах. Мы приветствуем Pull Request'ы для внесения исправлений.

## Инструкция по компиляции

[Обратите внимание на техническую документацию](https://github.com/MatsuriDayo/nekoray/tree/main/docs)

## Пожертвование

Если этот проект окажется для вас полезным, вы можете помочь нам и дальше разрабатывать его, сделав пожертвование.

Пожертвования в размере до 50 долларов США можно сделать на странице «Пожертвовать сейчас».
[Список поддержавших](https://mtrdnt.pages.dev/donation_list). Если вы не были добавлены сюда, пожалуйста, свяжитесь с нами.

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`

## Авторы

Ядра:

- [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core) (< 3.10)
- [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri) (< 3.10)
- [MatsuriDayo/v2ray-core](https://github.com/MatsuriDayo/v2ray-core) (< 3.10)
- [XTLS/Xray-core](https://github.com/XTLS/Xray-core) (≥ 3.10)
- [MatsuriDayo/Xray-core](https://github.com/MatsuriDayo/Xray-core) (≥ 3.10)
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
