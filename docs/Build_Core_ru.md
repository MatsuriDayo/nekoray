## Компиляция nekoray_core и nekobox_core

### Структура директорий

```
  | nekoray
  |   go/cmd/*
  | Xray-core
  | sing-box-extra
  | sing-box
  | ......
```

### Компиляция

1. `bash libs/get_source.sh` (автоматически скачивать структуру директорий, автоматически checkout коммит)
2. `GOOS=windows GOARCH=amd64 bash libs/build_go.sh`

Можно посмотреть доступные GOOS и GOARCH в `libs/build_go.sh`.

Для неофициальной сборки не нужна компиляция `updater` `launcher`

### теги sing-box

Актуальные теги смотрите в файле `libs/build_go.sh`.