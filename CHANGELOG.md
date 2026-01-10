# 更新日志 / Changelog

ONScripter-jh for Nintendo Switch 的所有重要更新记录。

格式基于 [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/spec/v2.0.0.html)。

---

## [2.2.0] - 2025-01-11

### 新增 - 统一游戏浏览器

- ✅ **内置中文游戏浏览器** - 无需外部启动器，开箱即用
  - SDL2 实现的现代化游戏选择界面
  - 自动扫描 `sdmc:/onsemu/` 目录下的所有游戏
  - 支持方向键、摇杆、L/R/ZL/ZR 快速导航
  - 支持触屏操作（单击选择，双击启动）
  - 实时显示游戏数量和脚本文件信息
  - 滚动条指示器，支持大量游戏列表
- ✅ **全中文界面** - 所有UI元素汉化
  - 标题："ONScripter 游戏浏览器"
  - 游戏计数："共找到 X 个游戏"
  - 帮助信息："A: 选择游戏 | B/+: 退出 | ↑↓: 上下移动"
  - 触屏提示："单击选择，双击启动游戏"
- ✅ **视觉优化** - 现代化配色方案
  - 深蓝灰背景 `{25, 30, 40}`
  - 亮蓝选中高亮 `{45, 130, 220}`
  - 橙金强调色 `{255, 180, 50}`
  - 选中项左侧彩色竖条标记
  - 优化的文字间距，避免重叠
- ✅ **内置字体** - 集成 `default_font.ttf`（5.9MB）
  - 从 romfs 加载字体，无需外部字体文件
  - 支持中日英文字符显示
  - 大字体（32px）用于游戏标题
  - 小字体（24px）用于详细信息

### 移除 - 旧GUI依赖

- ❌ 移除 ONSBrowser 外部启动器依赖
- ❌ 移除 `return_path` 全局变量
- ❌ 移除 `DEFAULT_RETURN_PATH` 常量
- ❌ 移除 `envSetNextLoad()` 调用
- ❌ 移除 exefs 目录特殊处理
- ❌ 不再需要安装 NSP 前端

### 改进

- 🚀 **极简安装** - 单 NRO 文件部署
  - 复制 `ONScripter.nro` 到 `sdmc:/switch/ONScripter/` 即可
  - 不再需要复杂的 NSP 安装步骤
  - 不再需要单独的启动器程序
- 🚀 **退出优化** - 直接返回 HBMenu
  - `ons_exit()` 清理资源后正常退出
  - 不再尝试跳转到外部程序
- 🚀 **系统兼容** - 完美支持 Atmosphere 21.1.0
  - 更新 `main_thread_stack_size` 为 2MB
  - 优化内存布局和系统调用
  - 移除过时的 API 调用
- 🚀 **元数据更新**
  - 版本号：2.1.0 → 2.2.0
  - 作者：Wetor → ONScripter-jh Contributors
  - 标题保持：ONScripter

### 文件变更

- 新增：`source/GameBrowser.cpp` - 游戏浏览器实现
- 新增：`include/GameBrowser.h` - 游戏浏览器接口
- 新增：`romfs/font.ttf` - 内置字体文件（5.9MB）
- 修改：`source/main.cpp` - 集成浏览器逻辑
- 修改：`include/main.h` - 移除旧GUI声明
- 修改：`Makefile` - 添加 GameBrowser.o 编译

### 技术细节

- GameBrowser 类使用 SDL2 + SDL_ttf 渲染
- 每页显示 8 个游戏，支持无限滚动
- 游戏检测：扫描包含 `0.txt`, `00.txt`, 或 `nscript.dat` 的目录
- 布局参数：标题栏 110px，列表项 70px，底部帮助 100px
- 触屏双击时间窗口：300ms
- 字体路径优先级：`romfs:/font.ttf` → `/switch/ONScripter/font.ttf`

---

## [2.3.0] - 2025-01-06

### Added - GLES Renderer (CAS Sharpening)

- **GLES renderer with CAS (Contrast Adaptive Sharpening)** from OnscripterYuri
  - GPU-accelerated image sharpening for better visual quality when upscaling
  - Based on AMD FidelityFX CAS algorithm
  - Configurable via `--sharpness <0.0-1.0>` command line option
  - Automatic fallback to standard SDL rendering when sharpness is not set
- New source files:
  - `source/renderer/gles_renderer.h` - GLES renderer class header
  - `source/renderer/gles_renderer.cpp` - GLES renderer implementation
  - `source/renderer/gles2funcs.h` - GLES2 function declarations for dynamic binding
  - `source/renderer/shader/post_cas.h` - CAS fragment shader (GLSL ES 3.0)
- `USE_GLES` compile flag for enabling GLES renderer
- `GlesRenderer` class with pause/resume support for lifecycle management
- `render_view_rect` member for proper scaling calculations

### Changed

- Updated Makefile to include `source/renderer` in SOURCES directories
- ONScripter.h now includes GLES renderer forward declaration when USE_GLES is defined
- `calcRenderRect()` now updates GLES renderer constants when render rect changes
- `flushDirect()` uses GLES CAS renderer when sharpness is enabled
- `sharpness` member variable now uses NAN as default (unset state)

### Technical

- GLES 2.0/3.0 compatible shader implementation
- Nintendo Switch uses OpenGL ES via mesa/nouveau drivers
- Vertex buffer objects for efficient fullscreen quad rendering
- Dynamic GL function binding for desktop platform compatibility

## [2.2.0] - 2025-01-15

### Added - OnscripterYuri Feature Merge

- **UTF-8 script encoding support** (`--enc:utf8`) - Major feature from OnscripterYuri
  - Allows running games with UTF-8 encoded scripts
  - Use `iconv -f gbk -t utf8 0.txt -o 0.txt` to convert existing scripts
- **Arbitrary resolution support** (`--width`, `--height`) - Force custom window dimensions
- **GLES sharpness rendering** (`--sharpness <value>`) - GPU-based image sharpening
- **Fullscreen stretch mode** (`--fullscreen2`) - Fullscreen with aspect ratio stretch
- **Video disable option** (`--no-video`) - Skip video decoding for compatibility
- `force_utf8` flag in Coding2UTF16 class for UTF-8 mode detection
- `UTF8_N_BYTE` macro for UTF-8 byte length detection
- `stretch_mode`, `video_off`, `force_window_width`, `force_window_height`, `sharpness` member variables
- Global logging paths (`g_stdoutpath`, `g_stderrpath`) for file-based logging
- `auto_cast` template utility class from OnscripterYuri
- ONS_YURI_VERSION definition to track upstream version
- Feature flag macros (ONS_FEATURE_UTF8_SCRIPT, etc.)

### Changed

- Upgraded ONS_JH_VERSION from 0.7.6 to 0.8.0 (synced with OnscripterYuri)
- Reorganized command-line help into categories (Load, Render, Other options)
- `setFullscreenMode()` now accepts mode parameter (0=normal, 1=fullscreen, 2=stretch)
- `setWindowMode()` now properly resets fullscreen_mode flag
- Static strings in coding2utf16.cpp now have proper UTF-8 initialization
- Improved UTF-8 to UTF-16 conversion with 4-byte sequence handling
- Updated copyright headers to include yurisizuku (OnscripterYuri author)
- Command-line parser now supports `--enc:gbk` explicitly

### Technical

- Based on OnscripterYuri v0.7.6beta2 (October 2025)
- Maintained backward compatibility with existing GBK/SJIS scripts
- All new features are optional and don't affect default behavior

## [2.1.0] - 2025-01-15

### Added

- Modern C++17 support with updated language features
- Enhanced logging system with multiple log levels (DEBUG, INFO, WARNING, ERROR)
- RAII wrappers for video player resources (KitSourceGuard, KitPlayerGuard, TextureGuard)
- Utility functions: `startsWith`, `endsWith`, `strcasecmp_safe`, `strncpy_safe`, `fileExists`, `getFileExtension`
- Template utility functions: `clamp`, `min`, `max`
- Platform-specific macros (ONS_PLATFORM_NAME, ONS_DEFAULT_SAVE_DIR, ONS_PATH_SEPARATOR)
- Safe memory management macros (SAFE_DELETE, SAFE_DELETE_ARRAY, SAFE_FREE)
- Keyboard skip support for video playback (Escape and Space keys)
- Version components for programmatic access (ONS_NX_VERSION_MAJOR, etc.)
- Comprehensive documentation in header files

### Changed

- Upgraded C++ standard from C++11 to C++17
- Updated compiler architecture flags to support ARMv8-A CRC + Crypto extensions
- Improved linker flags with garbage collection for unused sections
- Enhanced video player with better error handling and resource management
- Modernized main.cpp with structured initialization and cleanup
- Updated ONScripter.json with expanded syscall support and kernel compatibility
- Improved README.md with better formatting, tables, and build instructions
- Refactored Utils.h with timestamp-based logging output
- Better organized Makefile with improved readability

### Fixed

- Potential memory leaks in video player through RAII patterns
- Missing null checks in string utility functions
- Improved error messages with more context
- FFmpeg API compatibility (ch_layout, swr_alloc_set_opts2)
- Lua 5.1 header includes
- Library linking order for static builds
- SDL_kitchensink CMake minimum version

### Technical

- Added `-fdata-sections` and `-Wl,--gc-sections` for smaller binary size
- Added warning flags (`-Wall`, `-Wextra`) with appropriate suppressions
- Updated min_kernel_version to 0x0 for broader firmware compatibility
- Increased highest_cpu_id from 2 to 3 for better multi-core utilization

## [2.0.0] - 2021-07-05

### Added

- English game support with SJIS encoding option
- Language switching via Plus (+) button in settings

### Fixed

- Fixed crash on firmware 12.0 and above
- Fixed game exit not returning to GUI properly

## [1.1.0] - 2019-10-03

### Fixed

- Fixed launcher crash when fewer than 5 games are installed
- Fixed save data corruption for some games

## [1.0.0] - 2019-09-XX

### Added

- Initial release of ONScripter-jh for Nintendo Switch
- Based on ONScripter-jh version 0.7.6
- SDL2 rendering support
- Video playback via SDL_kitchensink/FFmpeg
- Chinese (GBK) and Japanese (SJIS) encoding support
- OGG Vorbis audio support
- Lua scripting support
- ARM NEON SIMD optimizations
- ONSBrowser GUI launcher

### Features

- Full ONScripter script compatibility
- Save/Load functionality
- Fullscreen and windowed modes
- Font outline rendering
- Touch screen support
- Joy-Con controller support

---

## 版本历史总结

| 版本  | 日期       | 重点更新                                |
| ----- | ---------- | --------------------------------------- |
| 2.3.0 | 2025-01-06 | GLES 渲染器，CAS 锐化                   |
| 2.2.0 | 2025-01-11 | 统一游戏浏览器，全中文界面，系统 21.1.0 |
| 2.1.0 | 2025-01-15 | C++17，改进日志，现代化代码             |
| 2.0.0 | 2021-07-05 | 英文支持，固件 12.0+ 兼容               |
| 1.1.0 | 2019-10-03 | 修复启动器和存档问题                    |
| 1.0.0 | 2019-09    | 首次发布                                |

---

---

## 上游项目

- **ONScripter** by Ogapee - https://github.com/ogapee/onscripter
- **ONScripter-jh** by jh10001 - 增强分支
- **OnscripterYuri** by YuriSizuku - https://github.com/YuriSizuku/OnscripterYuri

## 贡献者

- **Ogapee** (ogapee@aqua.dti2.ne.jp) - ONScripter 原作者
- **jh10001** (jh10001@live.cn) - ONScripter-jh 维护者
- **YuriSizuku** (https://github.com/YuriSizuku) - OnscripterYuri 作者
- **Wetor** (makisehoshimi@163.com) - Switch 早期移植
- **ONScripter-jh Contributors** - 现代化更新与维护

## 许可证

本项目基于 GNU General Public License v2.0 开源。
详见 [COPYING](COPYING) 文件。
