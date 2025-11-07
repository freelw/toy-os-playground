# QEMU 自动化调试配置指南

## 概述

本项目配置了一键启动 QEMU + GDB 调试的完整工作流。按 **F5** 即可自动启动 QEMU 并开始调试 bootloader。

---

## 配置架构

```
按 F5
  ↓
launch.json (调试配置)
  ↓ preLaunchTask
tasks.json (启动 QEMU)
  ↓
start_qemu_vnc_dbg.sh (QEMU 脚本)
  ↓
GDB 连接到 localhost:1234
```

---

## 1. Tasks 配置 (tasks.json)

### Task: "Start QEMU Debug Server"

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `label` | `"Start QEMU Debug Server"` | 任务名称，被 launch.json 引用 |
| `type` | `"shell"` | 执行 shell 命令 |
| `command` | `"./start_qemu_vnc_dbg.sh"` | 启动 QEMU 的脚本 |
| `isBackground` | `true` | **后台运行**，不阻塞后续调试任务 |
| `cwd` | `"${workspaceFolder}"` | 在项目根目录执行 |

### 关键配置：problemMatcher

```json
"problemMatcher": {
    "background": {
        "activeOnStart": true,
        "beginsPattern": ".",
        "endsPattern": "."
    }
}
```

**作用**：告诉 VS Code 这是后台任务，立即返回控制权给调试器，不等待任务结束。

### 关键配置：presentation

```json
"presentation": {
    "reveal": "always",      // 总是显示终端
    "focus": false,          // 不夺取焦点
    "panel": "dedicated",    // 使用独立面板
    "showReuseMessage": false
}
```

**作用**：QEMU 输出在独立终端显示，不干扰调试面板。

---

## 2. Launch 配置 (launch.json)

### 配置名称
**Debug QEMU Bootloader (16-bit)**

### 完整配置详解

| 字段 | 值 | 说明 |
|------|-----|------|
| `type` | `"cppdbg"` | 使用 C/C++ 调试器（需安装 ms-vscode.cpptools） |
| `request` | `"launch"` | 启动调试会话 |
| `program` | `"/bin/ls"` | 占位符（远程调试不使用，必填） |
| `miDebuggerPath` | `"/usr/bin/gdb-multiarch"` | GDB 可执行文件路径 |
| `miDebuggerServerAddress` | `""` | 空值，在 setupCommands 中手动连接 |
| `preLaunchTask` | `"Start QEMU Debug Server"` | **调试前自动执行的任务** |
| `cwd` | `"${workspaceFolder}"` | 工作目录 |
| `MIMode` | `"gdb"` | 使用 GDB Machine Interface 协议 |

### setupCommands 详解

按执行顺序：

#### ① 设置反汇编风格
```json
{
    "description": "Set disassembly flavor",
    "text": "set disassembly-flavor intel",
    "ignoreFailures": true
}
```
- 使用 Intel 语法：`mov eax, ebx`（而非 AT&T：`movl %ebx, %eax`）
- 更符合 Intel 手册习惯

#### ② 设置架构
```json
{
    "description": "Set architecture to i386:x86-64",
    "text": "set architecture i386:x86-64",
    "ignoreFailures": true
}
```
- **为什么是 x86-64？**
  - QEMU 模拟器（qemu-system-x86_64）使用 x86-64 GDB 协议
  - GDB 协议返回 x86-64 寄存器格式（608字节）
  - 必须在连接前设置，否则会报错：`Remote 'g' packet reply is too long`
- **可以调试 16位代码吗？**
  - 可以！i386:x86-64 架构支持 16位/32位/64位代码
  - GDB 会自动识别当前 CPU 模式

#### ③ 连接远程目标
```json
{
    "description": "Connect to remote target",
    "text": "target remote localhost:1234",
    "ignoreFailures": false
}
```
- 连接到 QEMU 的 GDB 服务器（`-s` 参数启用）
- **必须在设置架构之后**连接
- `ignoreFailures: false` - 连接失败则中止调试

#### ④ 设置断点
```json
{
    "description": "Set breakpoint at 0x7c00",
    "text": "b *0x7c00",
    "ignoreFailures": false
}
```
- 0x7c00 是 BIOS 加载 bootloader 的标准地址
- `*` 表示绝对物理地址

### setupCommands 的执行者

**谁执行这些命令？**
- C/C++ 扩展（ms-vscode.cpptools）读取配置
- 启动 `gdb-multiarch --interpreter=mi`
- 通过 **GDB/MI 协议**（非普通 stdin）发送命令
- 根据 `ignoreFailures` 决定是否继续

**MI（Machine Interface）**：
- GDB 的机器可读协议
- 命令：`-gdb-set architecture i386:x86-64`
- 响应：`^done` 或 `^error,msg="..."`

---

## 3. QEMU 启动脚本

### start_qemu_vnc_dbg.sh

```bash
#!/bin/bash
( echo 'change vnc password' ; sleep 1 ; echo '0' ) | \
qemu-system-x86_64 \
  -vnc :0,password=on \
  -monitor stdio \
  -drive file=./a.vfd,index=0,if=floppy,format=raw \
  -boot a \
  -drive file=./disk1g.img,index=1,if=ide,format=raw \
  -s \    # GDB 服务器监听 tcp::1234
  -S      # CPU 暂停，等待 GDB 连接
```

**关键参数**：
- `-s`：启动 GDB 远程调试服务器，等价于 `-gdb tcp::1234`
- `-S`：CPU 启动后立即停止，等待调试器连接
- `-monitor stdio`：通过标准输入控制 QEMU
- `-vnc :0,password=on`：VNC 显示，密码为 "0"

---

## 使用指南

### 🚀 一键启动调试

1. **按 F5**（或点击调试面板的绿色三角）
2. 自动执行流程：
   - ✅ 启动 QEMU（后台运行）
   - ✅ GDB 连接到 localhost:1234
   - ✅ 设置断点 0x7c00
   - ✅ 停在 bootloader 入口

### 🎮 调试操作

| 操作 | 快捷键 | 说明 |
|------|--------|------|
| 单步执行（不进入函数） | F10 | Step Over |
| 单步执行（进入函数） | F11 | Step Into |
| 跳出函数 | Shift+F11 | Step Out |
| 继续运行 | F5 | Continue |
| 停止调试 | Shift+F5 | Stop |
| 重启调试 | Ctrl+Shift+F5 | Restart |

### 🔍 查看信息

**寄存器**：
- 调试面板 → Variables → Registers

**内存**：
- 打开 Debug Console（Ctrl+Shift+Y）
- 输入：`-exec x/16xb 0x7c00`（查看 0x7c00 的 16 字节）

**反汇编**：
- 调试面板 → Call Stack → 右键 → Disassembly

**表达式求值**：
- 调试面板 → Watch → 添加表达式
- 例如：`$eax`、`*(int*)0x7c00`

### 🛑 停止与重启

**停止调试**：
- 按 Shift+F5
- 注意：QEMU 进程仍在运行

**终止 QEMU**：
- 在 QEMU 终端面板按 Ctrl+C
- 或命令行：`pkill qemu-system-x86_64`

**重新调试**：
- 如果 QEMU 仍在运行：直接按 F5（会提示任务已在运行，选择重启）
- 如果已终止 QEMU：按 F5 自动重新启动

---

## 常见问题

### Q1: 报错 "Connection refused"

**原因**：QEMU 没有启动或 GDB 服务器未就绪

**解决**：
1. 检查 QEMU 终端是否有输出
2. 确认 `start_qemu_vnc_dbg.sh` 有执行权限：`chmod +x start_qemu_vnc_dbg.sh`
3. 手动启动 QEMU 测试：`./start_qemu_vnc_dbg.sh`

### Q2: 报错 "Remote 'g' packet reply is too long"

**原因**：GDB 架构设置不匹配 QEMU

**解决**：
- 确认 setupCommands 中有 `set architecture i386:x86-64`
- 确认架构设置在 `target remote` **之前**

### Q3: 为什么 program 是 /bin/ls？

**原因**：`cppdbg` 类型要求必填 `program` 字段

**解释**：
- 远程调试时，代码已在 QEMU 中运行
- `program` 仅用于加载符号信息（如果存在）
- `/bin/ls` 是系统自带文件，作为占位符

### Q4: 看不到 C 源码，只有汇编？

**原因**：bootloader 是纯二进制或汇编编写

**解决**：
- 如果有 C 源码，需要 ELF 格式且带 `-g` 调试符号
- 修改 `program` 指向带符号的 ELF 文件
- 纯汇编调试只能看反汇编代码

### Q5: 如何修改断点位置？

**方法 1**（推荐）：在源码/反汇编窗口点击行号左侧

**方法 2**：修改 launch.json
```json
{
    "text": "b *0x7c00",  // 改为其他地址
    ...
}
```

### Q6: 如何在调试时执行 GDB 命令？

**Debug Console** 中使用 `-exec` 前缀：
```
-exec info registers       // 查看寄存器
-exec x/16xb 0x7c00       // 查看内存
-exec disas 0x7c00,+32    // 反汇编
-exec set $eax=0x1234     // 修改寄存器
```

### Q7: preLaunchTask 每次都执行吗？

**是的**，每次按 F5 都会执行。

**优化**：
- 如果 QEMU 已在运行，可以暂时注释掉 `preLaunchTask`
- 或使用 Ctrl+Shift+D 打开调试面板，手动选择配置（会提示是否重启任务）

---

## 技术原理

### GDB/MI 协议

C/C++ 扩展与 GDB 的通信流程：

```
VS Code Extension
      ↓ (启动)
gdb-multiarch --interpreter=mi
      ↓ (发送 MI 命令)
-gdb-set disassembly-flavor intel
      ↓ (返回)
^done
      ↓ (发送)
-gdb-set architecture i386:x86-64
      ↓ (返回)
^done
      ↓ (发送)
-target-select remote localhost:1234
      ↓ (返回)
^connected,addr="...",func="??"
```

### 架构匹配原理

| QEMU | GDB 架构 | 寄存器格式 | 是否匹配 |
|------|----------|------------|---------|
| qemu-system-x86_64 | i8086 | 308 字节 | ❌ 报错 |
| qemu-system-x86_64 | i386 | 400+ 字节 | ❌ 报错 |
| qemu-system-x86_64 | i386:x86-64 | 608 字节 | ✅ 匹配 |

**为什么 i386:x86-64 可以调试 16位代码？**
- GDB 协议层面使用 x86-64 格式
- 但 GDB 会自动检测 CPU 当前模式（实模式/保护模式/长模式）
- 反汇编器根据 CS 段属性正确解析 16/32/64位指令

---

## 配置文件参考

### 最小化 launch.json

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug QEMU",
            "type": "cppdbg",
            "request": "launch",
            "program": "/bin/ls",
            "miDebuggerPath": "/usr/bin/gdb-multiarch",
            "preLaunchTask": "Start QEMU Debug Server",
            "setupCommands": [
                {
                    "text": "set architecture i386:x86-64"
                },
                {
                    "text": "target remote localhost:1234"
                },
                {
                    "text": "b *0x7c00"
                }
            ],
            "MIMode": "gdb"
        }
    ]
}
```

### 最小化 tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Start QEMU Debug Server",
            "type": "shell",
            "command": "./start_qemu_vnc_dbg.sh",
            "isBackground": true,
            "problemMatcher": {
                "background": {
                    "activeOnStart": true,
                    "beginsPattern": ".",
                    "endsPattern": "."
                }
            }
        }
    ]
}
```

---

## 扩展阅读

- [GDB 远程调试文档](https://sourceware.org/gdb/current/onlinedocs/gdb/Remote-Debugging.html)
- [GDB/MI 协议规范](https://sourceware.org/gdb/current/onlinedocs/gdb/GDB_002fMI.html)
- [QEMU 调试参数](https://www.qemu.org/docs/master/system/gdb.html)
- [VS Code 调试配置](https://code.visualstudio.com/docs/editor/debugging)
- [cpptools 配置参考](https://code.visualstudio.com/docs/cpp/launch-json-reference)

---

## 版本信息

- **配置版本**：2.0
- **更新日期**：2025-11-07
- **兼容环境**：Linux x86_64, gdb-multiarch, qemu-system-x86_64
