# QEMU Bootloader 调试配置说明

## 配置名称
**Debug QEMU Bootloader (16-bit)**

## 用途
用于调试运行在 QEMU 中的 16位 bootloader 代码（例如：0x7c00 处的引导扇区）

---

## 配置项详解

### 基础配置

| 字段 | 值 | 说明 |
|------|-----|------|
| `type` | `cppdbg` | 使用 C/C++ 调试器（需安装 ms-vscode.cpptools 扩展） |
| `request` | `launch` | 启动调试会话 |
| `program` | `/bin/ls` | 占位符（满足必填要求，实际不使用） |
| `miDebuggerPath` | `/usr/bin/gdb-multiarch` | 使用 gdb-multiarch 调试器 |
| `cwd` | `${workspaceFolder}` | 工作目录为项目根目录 |

### 调试命令（setupCommands）

按执行顺序：

#### 1. 设置反汇编风格
```gdb
set disassembly-flavor intel
```
- 使用 Intel 汇编语法（而非 AT&T 语法）
- 更易读：`mov eax, ebx` 而非 `movl %ebx, %eax`

#### 2. 设置架构
```gdb
set architecture i386:x86-64
```
- **为什么用 x86-64？** QEMU 模拟器（qemu-system-x86_64）使用 x86-64 GDB 协议
- 可以同时调试 16位/32位/64位代码
- 解决 "g packet reply too long" 错误

#### 3. 连接远程目标
```gdb
target remote localhost:1234
```
- 连接到 QEMU 的 GDB 服务器（`-s` 参数启用，监听 1234 端口）
- 必须在设置架构**之后**连接

#### 4. 设置断点
```gdb
b *0x7c00
```
- 在 0x7c00 设置断点（BIOS 加载 bootloader 的标准地址）
- `*` 表示绝对地址

---

## 使用步骤

### 1. 启动 QEMU（带 GDB 服务器）
```bash
qemu-system-x86_64 \
  -drive file=./a.vfd,index=0,if=floppy,format=raw \
  -boot a \
  -s \     # 启动 GDB 服务器（监听 tcp::1234）
  -S       # CPU 暂停，等待 GDB 连接
```

### 2. 启动调试
- 按 **F5** 或点击「运行和调试」
- 选择 "Debug QEMU Bootloader (16-bit)"
- GDB 自动连接并停在 0x7c00

### 3. 调试操作
- **单步执行**：F10（step over）、F11（step into）
- **继续运行**：F5
- **查看寄存器**：调试面板 → 变量 → 寄存器
- **查看内存**：调试控制台输入 `-exec x/16xb 0x7c00`

---

## 常见问题

### Q: 为什么 program 是 /bin/ls？
A: `cppdbg` 要求必填，但远程调试时不使用此字段，仅作占位符。

### Q: 如何修改断点位置？
A: 修改 setupCommands 中的 `"text": "b *0x7c00"` 为其他地址。

### Q: 报错 "Connection refused"？
A: 确保 QEMU 已启动且带 `-s -S` 参数。

### Q: 能看到 C 源码吗？
A: 如果 bootloader 是汇编编写的纯二进制，只能看到反汇编代码。需要 ELF 格式且带 `-g` 调试符号才能看源码。

---

## 扩展阅读

- QEMU 调试参数：`-s` = `-gdb tcp::1234`
- GDB 架构列表：`gdb` 中执行 `set architecture` 查看
- Intel vs AT&T 语法：[GDB 手册](https://sourceware.org/gdb/current/onlinedocs/gdb/)

