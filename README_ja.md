# NekoRay / PC 用 NekoBox

QtベースのクロスプラットフォームGUIプロキシ設定マネージャ (バックエンド：v2ray / sing-box)

Windows / Linuxに対応。

## ダウンロード

### GitHub Releases (Portable ZIP)

Portable 形式、インストーラーなし。Releases からコンパイル済みバイナリをダウンロードし、解凍してお使いください。

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[ダウンロード](https://github.com/Matsuridayo/nekoray/releases)

[ダウンロードするパッケージがわからない場合のインストール方法](https://github.com/MatsuriDayo/nekoray/wiki/Installation-package-description)

### Package

#### AUR

- [nekoray](https://aur.archlinux.org/packages/nekoray)
- [nekoray-git](https://aur.archlinux.org/packages/nekoray-git) [![AUR CI](https://github.com/MatsuriDayo/nekoray/actions/workflows/update-pkgbuild.yml/badge.svg)](https://github.com/MatsuriDayo/nekoray/actions/workflows/update-pkgbuild.yaml)

#### archlinuxcn

- [nekoray](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/nekoray)
- [nekoray-git](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/nekoray-git)

#### Scoop Extras

`scoop install nekoray`

## Changelog と Telegram チャンネル

https://t.me/Matsuridayo

## ホームページとドキュメント

https://matsuridayo.github.io

## プロキシ

- SOCKS (4/4a/5)
- HTTP(S)
- Shadowsocks
- VMess
- VLESS
- Trojan
- TUIC ( sing-box )
- NaïveProxy ( Custom Core )
- Hysteria ( Custom Core or sing-box )
- Hysteria2 ( Custom Core or sing-box )
- Custom Outbound
- Custom Config
- Custom Core

## サブスクリプション

- Raw: 広く使われているいくつかのフォーマット (Shadowsocks、Clash、v2rayN など)

## 動作パラメーター

[動作パラメーター](docs/RunFlags.md)

## Windows 実行

DLL が見つからない場合は、[Microsoft C++ ランタイムライブラリ](https://aka.ms/vs/17/release/vc_redist.x64.exe)をダウンロードしてインストールしてください

## Linux 実行

[Linux 実行チュートリアル](docs/Run_Linux.md)

## macOS

メンテナンス不足のため、macOS 版はダウンロードできなくなりました。

また、以下のリポジトリで非公式に更新版を入手することもできる。[非公式リリース](https://github.com/aaaamirabbas/nekoray-macos/releases)

以下のチュートリアルを参考に、自分でコンパイルすることもできます。FAQ については歴史的な Issue を参照し、修正については Pull Request を歓迎します。

## コンパイルチュートリアル

[技術ドキュメント](https://github.com/MatsuriDayo/nekoray/tree/main/docs)を参照して下さい

## 寄付

このプロジェクトがあなたのお役に立つのであれば、寄付をすることでプロジェクトの継続を支援することができます。

50US ドル以上の寄付をされた方は、[寄付リスト](https://mtrdnt.pages.dev/donation_list)にアバターを表示することができます。ここに追加されていない場合は、ご連絡ください。

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`

- また、[Google Play](https://play.google.com/store/apps/details?id=moe.nb4a) からも寄付金を購入することができます。アリペイと WeChat の購入オプションもありますので、TG チャンネルのトップをご覧ください。

## クレジット

Core:

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
