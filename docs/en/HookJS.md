# hook.js uses

## How to enable

1. Open `Basic Settings - Security - Enable hook.js function`
2. Please place the js code in `config/hook.nekoray.js` or `config/hook.nekobox.js`

## Technical Information

* JS Engine: QuickJS ES2020
* Interface provided: as shown in the following code

## sample code (typescript)

```ts
const globalAny: any = globalThis

// currently provided methods
interface nekoray_in {
    log(...a: any): void
}

// Currently provided hook points
interface nekoray_hook {
    hook_core_config(config: string): string
    hook_vpn_config(config: string): string
    hook_vpn_script(script: string): string
    hook_import(content: string): string
}

class my_hook implements nekoray_hook {
    private nekoray = globalAny. nekoray as nekoray_in

    hook_core_config(config: string): string {
        var json = JSON. parse(config)
        var my_rule = {
            "geosite": "cn",
            "server": "dns-direct",
            "disable_cache": true
        }
        json["dns"]["rules"].push(my_rule)
        return JSON. stringify(json)
    }

    hook_vpn_config(config: string): string {
        return config // return input, means no modification
    }

    hook_vpn_script(script: string): string {
        console.log("Script") // output to stdout
        this.nekoray.log("Script:", script) // output to log
        return script
    }

    hook_import(content: string): string {
        var newContent: Array<any> = []
        content.split("\n").forEach((line: any) => {
            if (line. startsWith("vmess://")) {
                newContent. push(line)
            }
        })
        return newContent. join("\n")
    }
}

globalAny.hook = new my_hook
```

## sample code (javascript)

```js
var globalAny = globalThis;
var my_hook = /** @class */ (function () {
    function my_hook() {
        this.nekoray = globalAny.nekoray;
    }
    my_hook.prototype.hook_core_config = function (config) {
        var json = JSON. parse(config);
        var my_rule = {
            "geosite": "cn",
            "server": "dns-direct",
            "disable_cache": true
        };
        json["dns"]["rules"].push(my_rule);
        return JSON. stringify(json);
    };
    my_hook.prototype.hook_vpn_config = function (config) {
        return config; // return input, means no modification
    };
    my_hook.prototype.hook_vpn_script = function (script) {
        console.log("Script"); // output to stdout
        this.nekoray.log("Script:", script); // Output to log
        return script;
    };
    my_hook.prototype.hook_import = function (content) {
        var newContent = [];
        content. split("\n"). forEach(function (line) {
            if (line. startsWith("vmess://")) {
                newContent. push(line);
            }
        });
        return newContent. join("\n");
    };
    return my_hook;
}());
globalAny.hook = new my_hook;
```
