#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/system_properties.h>
// 保存原函数指针
static int (*orig_is_loggable)(int prio, const char* tag, int default_prio);
// 替换后的__android_log_is_loggable函数
int __android_log_is_loggable(int prio, const char* tag, int default_prio) {
    // 读取动态配置的放行包名
    char allow_pkg[PROP_VALUE_MAX] = {0};
    __system_property_get("persist.sys.allow_log_pkg", allow_pkg);
    
    // 未配置时，默认拦截所有日志，和原模块行为一致
    if (allow_pkg[0] == '\0') {
        return 0;
    }
    // 读取当前进程的cmdline，获取应用包名（非uid模式）
    char cmdline[PROP_VALUE_MAX] = {0};
    FILE* f = fopen("/proc/self/cmdline", "r");
    if (f != NULL) {
        fread(cmdline, 1, sizeof(cmdline) - 1, f);
        fclose(f);
    }
    // 包名匹配，放行，调用原函数处理日志
    if (strcmp(cmdline, allow_pkg) == 0) {
        return orig_is_loggable(prio, tag, default_prio);
    }
    // 不匹配，拦截日志
    return 0;
}
// 构造函数，库加载时自动执行，获取原函数地址
__attribute__((constructor))
void log_filter_init() {
    orig_is_loggable = dlsym(RTLD_NEXT, "__android_log_is_loggable");
}
