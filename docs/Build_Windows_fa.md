کامپایل Nekoray در ویندوز

### git clone

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

### نصب Visual Studio

با نصب از وب سایت رسمی مایکروسافت، می توانید از نسخه های 2019 و 2022 استفاده کنید و محیط توسعه ++Win32 C را نصب کنید.

پس از نصب، می‌توانید `x64 Native Tools Command Prompt` را در منوی «شروع» پیدا کنید.

دستورات زیر این مقاله همه در این cmd اجرا می شوند. `cmake‍‍‍` `ninja` و ابزارهای دیگر را می توان با VS استفاده کرد.

### دانلود Qt SDK

نسخه ای که در حال حاضر توسط Windows Release استفاده می شود Qt 6.5.x است.

پس از دانلود و خارج کردن از حالت ظش، پوشه bin را به environment variable اضافه کنید.

#### بسته Qt مورد استفاده برای کامپایل نسخه (MSVC2019 x86_64) را دانلود کنید

https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/Qt6.5.0-Windows-x86_64-VS2022-17.5.5-20230507.7z

#### Official signed version of Qt 5.15.2 (optional, known memory leak bugs)

پکیج های `qtbase` `qtsvg` `qttools` را از اینجا دانلود کنید و آن را در همان دایرکتوری استخراج کنید.

https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5152/qt.qt5.5152.win64_msvc2019_64/

### کامپایل جزئی ++c

#### وابستگی های مورد نیاز ++C/C را کامپایل و نصب کنید

(You may have to hang the ladder for this step)

```shell
bash ./libs/build_deps_all.sh
```

در حال حاضر فقط اسکریپت های bash وجود دارد، بدون پردازش دسته ای یا پاورشل، اگر ویندوز همراه bash نیست، توصیه می شود خودتان آن را نصب کنید.

جزئیاتی مانند پارامترهای CMake مشابه لینوکس است. اگر سوالی دارید، لطفاً به داکیومنت [Build_Linux_fa.md](./Build_Linux_fa.md) مراجعه کنید.

#### کامپایل کد اصلی

لطفاً دستورات را با موقعیت QT Sdk خود جایگزین کنید.

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/path/to/qt/5.15.2/msvc2019_64 ..
ninja
```

بعد از اتمام کامپایل، فایل `nekoray.exe` را دریافت خواهید کرد.

سپس با اجرای دستور `windeployqt nekoray.exe`، فایل‌های مورد نیاز مانند DLL به صورت خودکار در دایرکتوری فعلی کپی می‌شوند.


### بخش کامپایل Go

لطفاً به [Build_Core_fa.md](./Build_Core_fa.md) مراجعه کنید.
