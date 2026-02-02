# ONScripter Yuri for Nintendo Switch

[![Build Nintendo Switch](https://img.shields.io/github/actions/workflow/status/YuriSizuku/OnscripterYuri/build_switch.yml?label=Switch%20Build&logo=nintendo-switch&style=flat-square)](https://github.com/YuriSizuku/OnscripterYuri/actions)
[![License](https://img.shields.io/badge/license-GPL--2.0-green.svg)](COPYING)
[![Platform](https://img.shields.io/badge/platform-Nintendo%20Switch-red.svg)]()

> 🎮 在 Nintendo Switch 上运行 NScripter/ONScripter 游戏的现代化模拟器

基于 [OnscripterYuri](https://github.com/YuriSizuku/OnscripterYuri) 移植，专门针对 Nintendo Switch 优化。

---

## ✨ 特性

- 🚀 **开箱即用** - 内置中文游戏浏览器，无需额外安装启动器
- 🎯 **最新系统支持** - 完美支持 Atmosphere 固件
- 🖼️ **现代化界面** - 全中文 UI，触屏支持，操作流畅
- ⚡ **高性能优化** - ARM NEON SIMD 加速，C++17 标准
- 🎬 **完整功能** - Lua 脚本、存档管理
- 📦 **极简安装** - 复制到 `switch` 目录即可使用

---

## 📥 快速开始

### 1️⃣ 安装模拟器

将 `onsyuri.nro` 复制到 SD 卡：

```
sdmc:/switch/
```
该目录下即可
### 2️⃣ 添加游戏

创建游戏目录并放入游戏文件：

```
sdmc:/onsemu/游戏名称/
  ├─ 0.txt 或 00.txt (必需)
  ├─ arc.nsa (资源包)
  └─ 其他游戏文件...
```

**注意**：不支持中文文件夹名称

### 3️⃣ 启动游戏

1. 从 HBMenu 打开 **onsyuri**
2. 游戏浏览器会自动显示 `onsemu` 文件夹中的所有游戏
3. 使用方向键或触屏选择游戏，按 A 键启动

**就这么简单！**

---

## 🎮 操作说明

### 游戏浏览器

| 操作     | 功能                 |
| -------- | -------------------- |
| ←→       | 左右选择游戏         |
| A        | 启动游戏             |
| B        | 关闭弹窗             |
| L        | 显示/隐藏帮助        |
| Y        | 详细信息             |
| X        | 资源查看             |
| R        | 快速滚动             |
| +        | 设置（占位）         |
| -        | 退出浏览器           |
| ZL / ZR  | 翻页                 |

### 游戏中

| 操作       | 功能                     |
| ---------- | ------------------------ |
| A          | 确认/前进                |
| B          | 取消/返回/右键菜单       |
| X          | 跳过文字                 |
| Y          | 自动模式                 |
| +          | 菜单                     |
| -          | 隐藏文字框               |
| L          | 回看历史                 |
| R          | 快进                     |
| 左摇杆     | 移动光标                 |
| L3 (按下)  | 切换鼠标自由移动模式     |
| 触摸屏     | 点击操作                 |

---

## 📋 系统要求

- ✅ Nintendo Switch（破解机）
- ✅ Atmosphere 自制固件
- ✅ 系统版本：9.0.0+
- ✅ SD 卡（用于存放游戏）

---

## 📂 目录结构示例

```
sdmc:/
├─ switch/
│  └─ onsyuri/
│     └─ onsyuri.nro              ← 模拟器主程序
│
└─ onsemu/                        ← 游戏目录
   ├─ eden/                       ← 游戏1
   │  ├─ 0.txt
   │  ├─ arc.nsa
   │  └─ ...
   │
   ├─ umineko/                    ← 游戏2
   │  ├─ 0.txt
   │  ├─ arc1.nsa
   │  └─ ...
   │
   ├─ stdout.txt                  ← 日志文件（自动生成）
   └─ stderr.txt                  ← 错误日志（自动生成）
```

---

## 🛠️ 开发编译

### 环境准备

```bash
# 安装 devkitPro
# macOS: brew install devkitpro-pacman
# Linux: https://devkitpro.org/wiki/Getting_Started

# 安装依赖
sudo dkp-pacman -S switch-dev switch-sdl2 switch-sdl2_ttf \
                  switch-sdl2_image switch-sdl2_mixer \
                  switch-liblua51 switch-freetype switch-harfbuzz
```

### 编译步骤

```bash
# 克隆项目
git clone https://github.com/YuriSizuku/OnscripterYuri.git
cd OnscripterYuri

# 编译 Switch 版本
make -f Makefile.switch -j$(nproc)
```

输出文件：`onsyuri.nro`

---

## 🎮 游戏资源

### 获取游戏

ONScripter 可运行大部分使用 NScripter 引擎的游戏：

- 从网上寻找 ONScripter 兼容的游戏资源包
- 不区分平台，只要是 ONScripter 格式即可
- 支持中文、日文、英文游戏

### 验证游戏完整性

游戏文件夹必须包含以下文件之一：

- `0.txt` 或 `00.txt`（脚本文件）
- `nscript.dat`（编译后的脚本）

可能包含的资源文件：

- `*.nsa` - NSA 压缩档案
- `*.sar` - SAR 压缩档案
- `*.jpg`, `*.png` - 图片资源
- `*.ogg`, `*.mp3` - 音频资源

---

## 🐛 故障排除

### 游戏无法识别

1. 检查游戏文件夹中是否存在 `0.txt` 或 `00.txt`
2. 确认文件夹放在 `sdmc:/onsemu/` 目录下
3. 游戏文件夹名称避免使用特殊字符

### 游戏无法启动

1. 查看 `sdmc:/onsemu/stdout.txt` 和 `stderr.txt` 日志
2. 尝试在其他平台的 ONScripter 测试同一游戏包
3. 确认游戏资源完整

### 其他问题

- 检查系统固件版本是否为 9.0.0+
- 确保 SD 卡有足够空间
- 重新下载最新版本模拟器

---

## 📖 相关链接

| 项目               | 链接                                                      |
| ------------------ | --------------------------------------------------------- |
| OnscripterYuri     | https://github.com/YuriSizuku/OnscripterYuri              |
| 原版 ONScripter    | https://onscripter.osdn.jp/                               |
| ONScripter-jh      | https://bitbucket.org/jh10001/onscripter-jh               |

---

## 📝 许可证

本项目基于 **GNU General Public License v2.0** 开源。

详见 [COPYING](COPYING) 文件。

---

## 👥 贡献者

- **Ogapee** - ONScripter 原作者
- **jh10001** - ONScripter-jh 维护者
- **YuriSizuku** - OnscripterYuri 开发者

---

## 💬 问题反馈

遇到问题？请在 [GitHub Issues](https://github.com/YuriSizuku/OnscripterYuri/issues) 提交反馈。

**提交时请包含**：

- Switch 系统版本
- Atmosphere 版本
- 游戏名称
- 错误日志（`stdout.txt` 和 `stderr.txt`）
- 详细的复现步骤

---

**享受你的 Switch 视觉小说之旅！** 🎮✨
