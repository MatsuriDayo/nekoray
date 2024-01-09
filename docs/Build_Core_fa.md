## ساخت nekoray_core و nekobox_core

### ساختار دایرکتوری

```
  | nekoray
  |   go/cmd/*
  | Xray-core
  | sing-box-extra
  | sing-box
  | ......
```

### ساخت معمولی

1. `bash libs/get_source.sh `(دریافت خودکار ساختار دایرکتوری، خودکار checkout commit)
2. `GOOS=windows GOARCH=amd64 bash libs/build_go.sh`

برای مشاهده GOOS و GOARCH های پشتیبانی شده به `libs/build_go.sh` مراجعه کنید.

در ساخت غیررسمی، نیازی به کامپایل `updater` و `launcher` نیست.

### تگ‌های sing-box

برای مشاهده تگ‌های استفاده شده به `libs/build_go.sh` مراجعه کنید.







