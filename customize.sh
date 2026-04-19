```sh
#!/bin/sh
# 基于原源码修改后的动态LD\_PRELOAD方案安装脚本
# 无需修改原liblog.so，通过预加载库实现动态包名日志放行

# 创建目标目录
mkdir -p "${MODPATH}/system/lib64" || abort "Failed to create lib dir"

# 复制预编译好的liblogfilter.so到系统目录
cp -f liblogfilter.so "${MODPATH}/system/lib64/liblogfilter.so" || abort "Failed to copy filter lib"

# 创建magisk ld.preload配置，让magisk自动预加载我们的库
echo "/lib64/liblogfilter.so" > "${MODPATH}/ld.preload" || abort "Failed to create preload config"

# 设置SELinux上下文，和原模块一致
if \[ "$KSU" ]; then
    chcon "u:object\_r:system\_lib\_file:s0" "${MODPATH}/system/lib64/liblogfilter.so"
    ui\_print "Success to set SELinux context for filter lib"
fi

ui\_print "安装完成！"
ui\_print "使用方法："
ui\_print "1. 执行 setprop persist.sys.allow\_log\_pkg <你的应用包名> 来设置要放行日志的应用"
ui\_print "   例如：setprop persist.sys.allow\_log\_pkg com.example.myapp"
ui\_print "2. 设置后立即生效，无需重启"
ui\_print "3. 清空属性即可恢复全局拦截日志：setprop persist.sys.allow\_log\_pkg \\"\\""
ui\_print "Reboot to take effect!"
```
