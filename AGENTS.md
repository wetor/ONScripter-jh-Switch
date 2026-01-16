<!-- OPENSPEC:START -->

# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:

- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:

- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

# ONScripter-jh-Switch 项目说明

## 项目概述

Nintendo Switch 平台的 ONScripter/NScripter 游戏模拟器，基于 OnscripterYuri 移植。

## 目录结构

```
├── src/onsyuri/          # 主要源代码
│   ├── GameBrowser.cpp   # 游戏浏览器
│   ├── ONScripter*.cpp   # 核心引擎
│   └── onscripter_main.cpp # 程序入口
├── tests/                # 单元测试
├── romfs/                # 内置资源（字体、光标）
├── build_switch/         # 编译产物
└── Makefile.switch       # Switch 编译配置
```

## 编译命令

```bash
make -f Makefile.switch clean
make -f Makefile.switch -j8
```

输出：`onsyuri.nro`

## 运行测试

```bash
cd tests && make && ./run_input_tests && ./run_path_tests && ./run_game_browser_tests && ./run_screen_tests
```

## 关键路径

- 游戏目录：`sdmc:/onsemu/`
- 日志文件：`sdmc:/onsemu/stdout.txt`, `sdmc:/onsemu/stderr.txt`
- 存档目录：`sdmc:/onsemu/游戏名/save/`

## 按键映射

| 按键 | 功能         |
| ---- | ------------ |
| A    | 确认/前进    |
| B    | 取消/菜单    |
| X    | 跳过         |
| Y    | 自动模式     |
| L    | 回看历史     |
| R    | 快进         |
| L3   | 切换鼠标模式 |

## 开发注意事项

1. 修改按键映射在 `ONScripter_event.cpp`
2. 游戏浏览器逻辑在 `GameBrowser.cpp`
3. 程序入口和初始化在 `onscripter_main.cpp`
4. 不要修改 `build_switch/` 下的文件，它们是编译产物
