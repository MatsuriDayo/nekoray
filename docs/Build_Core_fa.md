## ساختن (بیلد) nekobox_core و nekoray_core

###ساختن (بیلد) آسان 

۱. در Terminal به روت پروژه Nekoray در دستگاه خود بروید.

اگر تا اینجا با مستندات پیش آمده باشید، کافی است دستور زیر را در Terminal وارد کنید و دکمه Return را بفشارید.
```shell
cd .
```
<br>

۲. در همین مسیر`v2ray-core` را از [این](https://github.com/MatsuriDayo/v2ray-core) منبع دریافت کنید.

برای این می توانید دستور زیر را در `Terminal` وارد کنید و دکمه `Return` را بفشارید.
```shell
git clone https://github.com/MatsuriDayo/v2ray-core.git
``` 
<br>

۳. دستورهای زیر را به ترتیب در `Terminal` وارد کنید و دکمه `Return` را بفشارید.
```shell
cd go/cmd/nekoray_core
go build
```
اگر پس از دستور دوم با خطای `zsh: command not found: go` رو به رو شدید به این معنی است که `go` روی دستگاه شما نصب نیست. برای نصب `go` دستور زیر را در `Terminal` وارد کنید و دکمه `Return` را بفشارید.
```shell
brew install go
```
پس از نصب شدن `go` باز دستور `go build` را در `Terminal` اجرا کنید.

در پایان این گام، فایل `nekoray_core` در مسیر زیر ساخته خواهد شد.

`مسیر پروژه Nekoray/go/cmd/nekoray_core`

<br> 

۴. دستورهای زیر را به ترتیب در `Terminal` وارد کنید و دکمه `Return` را بفشارید.
```shell
cd ..
cd nekobox_core
go build
```
در پایان این گام، فایل `nekobox_core` در مسیر زیر ساخته خواهد شد.

<br>

- برای موارد خاص به`libs/get_source.sh`و  `libs/build_go.sh` رجوع کنید. 
- نیازی نیست این مراحل را برای پوشه `updater` انجام دهید. این سخن به این معنی است که نیازی نیست `launcher` را بسازید (بیلد بگیرید).

<br>

### ساختن (بیلد) معمولی


۱. در Terminal به روت پروژه Nekoray در دستگاه خود بروید.

اگر تا اینجا با مستندات پیش آمده باشید، کافی است دستور زیر را در Terminal وارد کنید و دکمه Return را بفشارید.
```shell
cd .
```
<br>

۲. در همین مسیر`v2ray-core` را از [این](https://github.com/MatsuriDayo/v2ray-core) منبع دریافت کنید.

برای این می توانید دستور زیر را در `Terminal` وارد کنید و دکمه `Return` را بفشارید.
```shell
git clone https://github.com/MatsuriDayo/v2ray-core.git
``` 
<br>

۳. دستور زیر را در `Terminal`وارد کنید و دکمه `Return`را بفشارید.
```shell
GOOS=windows GOARCH=amd64 bash libs/build_go.sh
```

- برای دریافتن `GOOS` و `GOARCH` پشتیبانی شده به `libs/build_go.sh`مراجعه کنید.

<br>

### تگ های sing-box

بوای تگ های خاص به `libs/build_go.sh` رجوع کنید.