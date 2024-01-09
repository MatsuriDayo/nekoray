کامپایل کردن nekoray در لینوکس
## کلون کردن کد با گیت
```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```
## روش کامپایل ساده

پیش‌نیازها:

1. وابستگی‌های C++ به نام‌ها: `protobuf yaml-cpp zxing-cpp` باید با مدیریت بسته نصب شده باشند و با نسخه‌های مورد نیاز سازگار باشند.
2. باید `qtbase`، `qtsvg`، `qttools`، `qtx11extras` نصب شده باشد.
3. Qt با نسخه `5.12.x` یا `5.15.x` باید نصب شده باشد.
4. سیستم باید `x86-64-linux-gnu` باشد.
```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```
پس از کامپایل، فایل nekoray به دست می‌آید.

فایل‌های فشرده را از Release استخراج کرده و nekoray را جایگزین کنید، سپس launcher را حذف کنید.

## روش دیگر کامپایل (پیشرفته)


### پارامترهای CMake

| پارامتر CMake         | مقدار پیش‌فرض              | توضیح                   |
|-------------------|-------------------|-----------------------|
| QT_VERSION_MAJOR  | 5                 | نسخه QT                 |
| NKR_NO_EXTERNAL   |                   | شامل وابستگی‌های خارجی C/C++ (همه زیرمجموعه‌ها) نشود |
| NKR_NO_YAML       |                   | شامل yaml-cpp نشود      |
| NKR_NO_QHOTKEY    |                   | شامل qhotkey نشود        |
| NKR_NO_ZXING      |                   | شامل zxing نشود            |
| NKR_NO_GRPC       |                   | شامل gRPC نشود           |
| NKR_NO_QUICKJS    |                   | شامل quickjs نشود          |
| NKR_PACKAGE       |                   | نسخه‌ی بسته را کامپایل کنید (aur)  |
| NKR_PACKAGE_MACOS |                   | نسخه‌ی macOS را کامپایل کنید          |
| NKR_LIBS          | ./libs/deps/built | مسیر جستجوی وابستگی‌ها               |
| NKR_DISABLE_LIBS  |                   | وابستگی‌های NKR_LIBS غیرفعال شود          |

1. مقدار `NKR_LIBS‍‍‍‍‍` به `CMAKE_PREFIX_PATH` اضافه می‌شود.

2. در صورت فعال بودن `NKR_PACKAGE`، مقدار پیش‌فرض `NKR_LIBS` برابر با `./libs/deps/package` است، جزئیات وابستگی‌ها در `build_deps_all.sh` قابل مشاهده است.
3. در صورت فعال بودن `NKR_PACKAGE_MACOS` یا `NKR_PACKAGE`، برنامه از دایرکتوری appdata برای ذخیره تنظیمات و سایر عملیات استفاده می‌کند و قابلیت به‌روزرسانی خودکار غیرفعال خواهد شد.

### C++

اگر توزیع شما وابستگی‌های C++ مورد نظر یا نسخه مورد نظر را نداشته باشد، می‌توانید به اسکریپت کامپایل `build_deps_all.sh` مراجعه کرده و خودتان کامپایل کنید.

پیش‌نیازها: Qt با نسخه `5.12.x` یا `5.15.x` باید نصب شده باشد.

#### کامپایل و نصب وابستگی‌های C/C++

(ممکن است به اتصال VPN نیاز داشته باشد)

```shell
./libs/build_deps_all.sh
```

#### کامپایل برنامه اصلی

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

پس از کامپایل، فایل `nekoray` به دست می‌آید.

### کامپایل بخش Go
لطفاً به [Build_Core_fa.md](./Build_Core_fa.md) مراجعه کنید.
