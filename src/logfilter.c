```C#
#define \_GNU\_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/system\_properties.h>

// 保存原函数指针
static int (\*orig\_is\_loggable)(int prio, const char\* tag, int default\_prio);

// 替换后的\_\_android\_log\_is\_loggable函数
int \_\_android\_log\_is\_loggable(int prio, const char\* tag, int default\_prio) {
    // 读取动态配置的放行包名
    char allow\_pkg\[PROP\_VALUE\_MAX] = {0};
    \_\_system\_property\_get("persist.sys.allow\_log\_pkg", allow\_pkg);
    
    // 未配置时，默认拦截所有日志，和原模块行为一致
    if (allow\_pkg\[0] == '\\0') {
        return 0;
    }

    // 读取当前进程的cmdline，获取应用包名（非uid模式）
    char cmdline\[PROP\_VALUE\_MAX] = {0};
    FILE\* f = fopen("/proc/self/cmdline", "r");
    if (f != NULL) {
        fread(cmdline, 1, sizeof(cmdline) - 1, f);
        fclose(f);
    }

    // 包名匹配，放行，调用原函数处理日志
    if (strcmp(cmdline, allow\_pkg) == 0) {
        return orig\_is\_loggable(prio, tag, default\_prio);
    }

    // 不匹配，拦截日志
    return 0;
}

// 构造函数，库加载时自动执行，获取原函数地址
\_\_attribute\_\_((constructor))
void log\_filter\_init() {
    orig\_is\_loggable = dlsym(RTLD\_NEXT, "\_\_android\_log\_is\_loggable");
}
```
