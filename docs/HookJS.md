# hook.js 使用
## 如何启用

1. 打开 `基本设置 - 安全 - 启用 hook.js 功能`
2. js 代码请放置于 `config/hook.nekoray.js` 或 `config/hook.nekobox.js`

## 技术信息

* JS Engine: QuickJS ES2020
* 提供的接口：如下面代码所示

## 示例代码 (typescript)

```ts
const globalAny: any = globalThis

// 目前提供的方法
interface nekoray_in {
    log(...a: any): void
}

// 目前提供的 hook 点
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
        return config // 返回输入，表示不修改
    }

    hook_tun_script(script: string): string {
        console.log("Script") // 输出到 stdout
        this.nekoray.log("Script:", script) // 输出到日志
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

## 示例代码 (javascript)

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
        return config; // 返回输入，表示不修改
    };
    my_hook.prototype.hook_tun_script = function (script) {
        console.log("Script"); // 输出到 stdout
        this.nekoray.log("Script:", script); // 输出到日志
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
