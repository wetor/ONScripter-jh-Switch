#ifndef __MKDIR_SWITCH_H__
#define __MKDIR_SWITCH_H__

#if defined(SWITCH) || defined(__SWITCH__)
#include <switch.h>
#include <sys/stat.h>
#include <string.h>

// devkitPro 的 newlib libc 没有提供 mkdir 实现（libc.a 中无该符号），
// 这里用 libnx 的 fsdev 实现替代：
//   fsdevGetDeviceFileSystem("sdmc") 取 SD 卡文件系统句柄
//   fsFsCreateDirectory 创建目录（路径相对于该文件系统根）
// 支持 "sdmc:/xxx" 全路径与 "xxx" 相对路径两种写法。
static inline int mkdir_switch_impl(const char *path, mode_t mode)
{
    (void)mode;
    FsFileSystem *fs = fsdevGetDeviceFileSystem("sdmc");
    if (fs == NULL) return -1;
    const char *rel = path;
    if (strncmp(path, "sdmc:/", 6) == 0) rel = path + 6;
    return fsFsCreateDirectory(fs, rel) == 0 ? 0 : -1;
}
#define mkdir mkdir_switch_impl
#endif

#endif
