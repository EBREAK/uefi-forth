# UEFI Forth

一个直接在 UEFI 固件上运行的原生 Forth 解释器，采用事件驱动的协作式多任务架构。

## 概述

UEFI Forth 是一个独立的 EFI 应用程序。它无需任何操作系统即可运行，并利用 UEFI 的原生事件系统处理 I/O 和任务调度。

### 核心特性

- **事件驱动架构** — 使用 UEFI 的 `WaitForEvent` 系统。所有 I/O（键盘输入、定时器等）均通过 UEFI 事件和回调处理。当没有待处理事件时，CPU 在 `WaitForEvent` 内部阻塞，实现零忙等待 — 这天然具有节能优势，固件可将 CPU 置于空闲状态（如 x86 的 C-state），直到事件触发。
- **协作式多任务** — 多个 Forth 任务在循环链表中运行。任务在等待 I/O 时主动让出 CPU，事件触发后恢复执行。
- **轻量级协程** — 使用 GCC 计算 goto（`&&label` / `goto *save`）在内层解释器中实现高效的协程让出/恢复。一个让出的任务从 `forth_run` 返回，并在下一次调度时从保存的标签位置重新进入。
- **单一共享字典** — 所有任务共享一个 8MB 的字典空间。编译过程设计是单线程的。
- **启动自检** — 系统启动时自动运行全面的自检套件，在进入交互式提示符前验证核心词汇、I/O 和解释器功能。

## 架构

### 任务调度

每个 `forth_context` 拥有独立的参数栈、返回栈和指令指针。任务组成一个循环链表。`forth_run_all()` 在每个调度周期遍历链表一次，运行每个任务直到它让出（等待 I/O）或停止（错误/关机）。

### 内层解释器

内层解释器采用直接线程码，通过 `switch` 语句分派操作码。控制流词汇（`BRANCH`、`ZBRANCH`、`CALL`、`EXIT`）直接操作指令指针。

### 内存布局

- **字典**: 8 MiB 运行时分配的 RAM（`FORTH_RAM_SIZE`）
- **单元大小**: `uintptr_t`（在 UEFI x86_64 上为 8 字节）
- **栈深度**: 每个任务的参数栈和返回栈均为 128 个单元

## Forth 模型

| 特性 | 说明 |
|------|------|
| 单元大小 | `uintptr_t`（x86_64 上为 8 字节） |
| 栈深度 | 每个任务 128 个单元 |
| 字符编码 | UTF-16（`CHAR16`），与 UEFI 规范一致 |
| 数字格式 | 仅十六进制，`$` 前缀（如 `$1A`、`$FF`、`$DEADBEEF`） |
| 执行模型 | 线程码 — 大多数高级词汇体是一系列 XT（执行令牌）指针 |
| 字典 | 单一共享字典，所有任务共享同一个词汇列表 |

### 核心词汇集

**栈操作**: `DROP`、`DUP`、`SWAP`、`NIP`、`OVER`、`PICK`、`DEPTH`、`ROT`、`-ROT`、`2DROP`、`2DUP`、`2SWAP`、`2OVER`、`2ROT`、`2-ROT`、`>R`、`R>`

**算术与逻辑**: `+`、`-`、`*`、`/`、`MOD`、`1+`、`1-`、`2*`、`2/`、`LSHIFT`、`RSHIFT`、`AND`、`OR`、`XOR`、`INVERT`、`NEGATE`

**比较运算**: `=`、`<>`、`<`、`>`、`U<`、`U>`、`0=`、`0<>`

**内存访问**: `C@`、`W@`、`L@`、`X@`、`C!`、`W!`、`L!`、`X!`、`CMOVE`、`WMOVE`、`LMOVE`、`XMOVE`、`CFILL`、`WFILL`、`LFILL`、`XFILL`

**字典与编译**: `HERE`、`PAD`、`ALLOT`、`C,`、`W,`、`L,`、`X,`、`LATEST`、`FIND`、`WORD-NEW`、`XT>WNAME`、`XT>WNLEN`、`XT>PREV`、`XT>BODY`、`XT>WDESC`、`XT>WDLEN`、`BLEN@`、`BLEN!`

**控制流**: `IF`、`THEN`、`BEGIN`、`AGAIN`、`UNTIL`、`BRANCH`、`ZBRANCH`、`EXIT`、`CALL`、`EXECUTE`

**解释器状态**: `[`、`]`、`COMPILE?`、`IMMEDIATE?`、`IMMEDIATE`

**常量**: `TRUE`、`FALSE`、`$0`–`$F`

**系统词汇**: `NOOP`、`HALT`、`DP`、`UP@`、`STA@`、`PSP-RST`、`DBGOFF`、`DBGON`

### I/O 词汇

**控制台**: `WEMIT`、`WKEY`、`WTYPE`、` .`（点）、`SPACE`、`CR`、`.S`

**十六进制格式化**: `XHEX.`、`NHEX.`、`NUM4BIT>WHEX`

**定时**: `DELAYUS`、`DELAYMS`、`DELAYS`

**内存转储**: `CDUMP`、`WDUMP`、`LDUMP`、`XDUMP`

### UEFI 图形输出协议 (GOP) 词汇

UEFI Forth 通过 UEFI GOP 提供内置图形原语：

**模式控制**: `GOP-INIT`、`GOP-CNT`、`GOP-SEL`、`GOP-CUR`、`GOP-MODE-CNT`、`GOP-MODE-SEL`、`GOP-MODE-CUR`、`GOP-HRES`、`GOP-VRES`

**绘图**: `GOP-PLOT`、`GOP-SOLID`、`GOP-HLINE`、`GOP-VLINE`、`GOP-FRAME`

**颜色**: `>PIXEL`、`>COLOR`、`PIXEL-WHITE`、`PIXEL-BLACK`、`PIXEL-RED`、`PIXEL-GREEN`、`PIXEL-BLUE`、`PIXEL-YELLOW`、`PIXEL-MAGENTA`、`PIXEL-CYAN`、`PIXEL-GRAY`

**便捷词汇**: `GOP-BLANK`

示例 — 将屏幕清空为红色：
```forth
GOP-INIT DROP
PIXEL-RED $0 $0 GOP-HRES GOP-VRES GOP-SOLID
```

### 定义新词汇

```forth
: SQUARE DUP * ;
$5 SQUARE .   ( 输出 $19 )
```

### 注释

```forth
( 这是括号注释 )
\ 这是行注释
```

### 字符串

```forth
PAD W" HELLO" WTYPE
```

## 构建

### 依赖

- 支持 `x86_64-pc-win32-coff` 目标的 `clang`
- `lld-link`（LLVM 链接器）
- GNU-EFI（作为 Git 子模块包含）
- `genimage`（用于创建磁盘镜像）
- QEMU（用于测试）
- OVMF 固件（默认路径 `/usr/share/ovmf/x64/OVMF.4m.fd`）

### 构建步骤

```bash
# 克隆并包含子模块
git clone --recursive <仓库地址>
cd uefi-forth

# 构建 EFI 应用程序
make

# 创建可启动磁盘镜像
make image

# 在 QEMU 中运行
make qemu
```

### 构建输出

- `BOOTX64.EFI` — EFI 应用程序
- `images/disk.img` — 带有 VFAT EFI 分区的可启动 GPT 磁盘镜像
- `images/disk.img.lz4` — 压缩后的磁盘镜像

## 项目结构

| 文件 | 说明 |
|------|------|
| `main.c` | UEFI 入口点、事件循环、任务调度粘合层 |
| `forth.c` | Forth 核心引擎：内层解释器、字典、任务管理、内存原语 |
| `forth.h` | 主头文件：操作码、上下文结构体、词汇结构体、宏 |
| `forth_core.c` | 内置原语词汇定义（栈、算术、内存） |
| `forth_io.c` | I/O 词汇定义（控制台、十六进制格式化、定时、转储） |
| `forth_interpret.c` | 解释器/编译器词汇（`FIND`、 `:`、 `;`、 `IF`、`BEGIN` 等） |
| `forth_gop.c` | UEFI 图形输出协议词汇 |
| `forth_test.c` / `*_test.c` | 启动时运行的自检套件 |
| `fifo.c` / `fifo.h` | 用于事件缓冲的无锁单生产者单消费者 FIFO（8/16/32/64 位变体） |
| `debug.c` / `debug.h` | 通过 UEFI 控制台的调试输出 |
| `wstring.c` / `wstring.h` | UTF-16 字符串工具函数 |
| `makefile` | 构建系统 |
| `genimage.cfg` | `genimage` 的磁盘镜像布局配置 |
| `gnu-efi/` | GNU-EFI 库（子模块） |

## 自检

启动时，解释器自动运行全面的自检，验证以下内容：
- 栈操作（`DUP`、`SWAP`、`ROT`、`PICK` 等）
- 算术和逻辑运算（`+`、`-`、`*`、`/`、`AND`、`OR`、`XOR` 等）
- 内存访问（`C@`、`W@`、`L@`、`X@`、`C!` 等）
- 字典操作（`HERE`、`ALLOT`、`C,`、`W,` 等）
- I/O 原语（`WTYPE`、`NUM4BIT>WHEX`）
- 解释器（`FIND`、`NUMBER?`、`>NUMBER`、`WIB`/`WIN`）

测试使用 `=CHK`（断言相等）和 `PZCHK`（断言参数栈为空）。任何失败都会使解释器停止并输出诊断信息。

自检成功后，系统打印欢迎信息（MOTD）并启动交互式解释器循环。

## 局限性

- 协程让出/恢复使用 GCC 特有的compund goto
- 无任务删除功能（任务只能创建，不能销毁）
- 全局编译栈 — 同一时间只有一个任务可以编译
- 所有任务共享单一字典
- 数字仅支持十六进制输入（`$` 前缀），不支持十进制
- 不支持浮点数

## 许可证

MIT
