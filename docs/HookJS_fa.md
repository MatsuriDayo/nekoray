# استفاده از فایل hook.js
## فعالسازی

1. `Basic Settings - Security - Enable hook.js function` را روشن کنید
2. کد JS را در `config/hook.nekoray.js` یا `config/hook.nekobox.js` قرار دهید.

## Technical Information

* JS engine: QuickJS ES2020.
* Provided interfaces: as shown in the following code

## کد نمونه (typescript)

```ts
const globalAny: any = globalThis

// روش‌های ارائه‌شده در حال حاضر
interface nekoray_in {
    log(...a: any): void
}

// نقاط hook ارائه‌شده در حال حاضر
interface nekoray_hook {
    hook_core_config(config: string): string
    hook_tun_config(config: string): string
    hook_tun_script(script: string): string
    hook_import(content: string): string
}

class my_hook implements nekoray_hook {
    private nekoray = globalAny.nekoray as nekoray_in

    hook_core_config(config: string): string {
        var json = JSON.parse(config)
        var my_rule = {
            "geosite": "cn",
            "server": "dns-direct",
            "disable_cache": true
        }
        json["dns"]["rules"].push(my_rule)
        return JSON.stringify(json)
    }

    hook_tun_config(config: string): string {
        return config // بازگشت ورودی، نشان دهنده عدم تغییر
    }

    hook_tun_script(script: string): string {
        console.log("Script") // خروجی به stdout
        this.nekoray.log("Script:", script) // خروجی به لاگ
        return script
    }

    hook_import(content: string): string {
        var newContent: Array<any> = []
        content.split("\n").forEach((line: any) => {
            if (line.startsWith("vmess://")) {
                newContent.push(line)
            }
        })
        return newContent.join("\n")
    }
}

globalAny.hook = new my_hook
```

## کد نمونه (javascript)

```js
var globalAny = globalThis;
var my_hook = /** @class */ (function () {
    function my_hook() {
        this.nekoray = globalAny.nekoray;
    }
    my_hook.prototype.hook_core_config = function (config) {
        var json = JSON.parse(config);
        var my_rule = {
            "geosite": "cn",
            "server": "dns-direct",
            "disable_cache": true
        };
        json["dns"]["rules"].push(my_rule);
        return JSON.stringify(json);
    };
    my_hook.prototype.hook_tun_config = function (config) {
        return config; // بازگشت ورودی، نشان دهنده عدم تغییر
    };
    my_hook.prototype.hook_tun_script = function (script) {
        console.log("Script"); // خروجی به stdout
        this.nekoray.log("Script:", script); // خروجی به لاگ
        return script;
    };
    my_hook.prototype.hook_import = function (content) {
        var newContent = [];
        content.split("\n").forEach(function (line) {
            if (line.startsWith("vmess://")) {
                newContent.push(line);
            }
        });
        return newContent.join("\n");
    };
    return my_hook;
}());
globalAny.hook = new my_hook;
```
