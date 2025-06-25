# PixtronVM - 轻量级类型安全栈式虚拟机

<div align="center">
    <img src="logo.svg" alt="PixtronVM Logo" width="200">
    <p><em>高效、安全、轻量级的字节码执行引擎</em></p >
</div>

**PixtronVM** 是一款专为资源受限环境设计的静态类型栈式虚拟机，通过显式类型标注和优化的字节码执行，在嵌入式系统和边缘计算场景中提供安全高效的代码执行能力。基于C11标准构建，支持CMake
3.10+构建系统，确保广泛的平台兼容性。

## 设计哲学

- **类型安全为先**：静态类型系统在编译期和加载期进行类型验证
- **性能与资源平衡**：优化指令调度减少运行时开销
- **安全隔离**：内置沙箱机制确保执行环境隔离
- **可扩展性**：模块化架构支持自定义扩展
- **标准兼容**：基于C11和CMake 3.10构建，确保广泛兼容性

## 📂 项目结构

```bash
PixtronVM/
├── assembler/              # Java实现的字节码汇编器
│   ├── src/
│   │   ├── main/java/      # 汇编器核心代码
│   │   │   ├── conf/       # 配置和元数据结构
│   │   │   ├── lexer/      # 词法分析器
│   │   │   ├── parser/     # 语法分析器
│   │   │   └── util/       # 工具类
│   │   └── test/           # 汇编器单元测试
├── include/                # C头文件
│   ├── api/                # 公共API接口
│   ├── engine/             # 虚拟机核心头文件
├── lib/                    # 标准库实现
│   ├── Math.klass          # 数学库字节码
│   ├── System.c            # 系统原生函数
├── src/                    # C源文件
│   ├── VM.c                # 虚拟机主入口
│   ├── Engine.c            # 指令执行引擎
│   ├── Memory.c            # 内存管理
│   ├── FFI.c               # 外部函数接口
│   └── ...                 # 其他核心组件
├── tests/                  # C测试套件
├── example/                # 示例程序
├── CMakeLists.txt          # 主构建配置
└── config.h.in             # 配置模板
```

## ✨ 核心特性

### 🛡️ 类型安全架构

- 显式类型标注的字节码格式
- 加载期类型完整性验证
- 运行时类型安全检查
- 支持基础数据类型：`int8`/`int16`/`int32`/`int64`/`double`/`bool`/`string`

### ⚡ 性能优化

- 直接线程代码(DTC)执行引擎
- 零开销指令分派机制
- 寄存器式栈缓存优化
- 指令预取流水线

### 🔒 安全沙箱

- 内存访问边界检查
- 指令执行计数限制
- 隔离堆栈执行环境
- 系统调用拦截层

### 📦 轻量化设计

- 核心引擎 < 50KB (ARM Cortex-M)
- 无动态内存分配
- 单文件头文件集成

## 🚀 快速开始

Hello World示例
创建 TString.s 汇编文件：

```asm
@namespace TString

@import { println } from System

@constant "Hello, World!"  # 定义字符串常量

@func main(): void
    %locals 0    # 无局部变量
    %stack 1     # 操作数栈深度1
    
    ldc.str 0    # 加载常量索引0的字符串
    call println # 调用打印函数
    ret          # 返回
@end
```

编译并运行：

```bash
# 编译汇编文件
./you/build/path/assembler -o /you/klass/path/ TString.s

# 执行字节码
./build/bin/pixtronvm /you/klass/path TString

# 输出: Hello, World!
```

## 🧩 系统架构

```mermaid
graph LR
    A[汇编源文件.s] --> B[Java汇编器]
    B --> C[字节码.klass]
    C --> D[PixtronVM加载器]
    D --> E[类型验证器]
    E --> F[执行引擎]
    F --> G[安全沙箱]
    G --> H[原生函数绑定]
    H --> I[系统资源]

    subgraph 宿主系统
        I[文件/网络/硬件]
    end
```

## 📖 开发指南

FFI外部函数接口

基本操作类型

```c
// 基本FFI操作（无参数/返回值）
typedef void (*KniBaseOperation)(RuntimeContext *context);

// 带返回值的FFI操作
typedef void (*KniResultOperation)(RuntimeContext *context, KniValue *result);
```

异常处理

```c
// 抛出格式化异常
extern void pvm_kni_throw_exception(RuntimeContext *context, char *fmt, ...);

// 使用示例：
void native_divide(RuntimeContext *ctx, KniValue *result) {
    double divisor = pvm_kni_get_double(ctx, 1);
    if (divisor == 0.0) {
        pvm_kni_throw_exception(ctx, "Division by zero at PC: %d", ctx->pc);
        return;
    }
    double quotient = pvm_kni_get_double(ctx, 0) / divisor;
    pvm_kni_set_double(result, quotient);
}
```

参数获取

```c
// 获取各种类型的参数
extern void pvm_kni_release_str(KniString **string);
extern int8_t pvm_kni_get_byte(RuntimeContext *context, uint16_t index);
extern int16_t pvm_kni_get_short(RuntimeContext *context, uint16_t index);
extern int32_t pvm_kni_get_int(RuntimeContext *context, uint16_t index);
extern int64_t pvm_kni_get_long(RuntimeContext *context, uint16_t index);
extern double pvm_kni_get_double(RuntimeContext *context, uint16_t index);
extern KniString *pvm_kni_get_str(RuntimeContext *context, uint16_t index);
extern KniValue *pvm_kni_get_object(RuntimeContext *context, uint16_t index);
```

结果设置

```c
// 设置各种类型的返回值
extern void pvm_kni_set_long(KniValue *result, int64_t value);
extern void pvm_kni_set_int(KniValue *result, int32_t value);
extern void pvm_kni_set_short(KniValue *result, int16_t value);
extern void pvm_kni_set_byte(KniValue *result, int8_t value);
extern void pvm_kni_set_double(KniValue *result, double value);
extern void pvm_kni_set_bool(KniValue *result, bool value);
```

完整FFI示例

Native 函数声明

```asm
; 声明命名空间
@namespace System
; 声明当前命名空间内native默认动态链接库
@library("PixotronVM")
; 声明native函数
@func @native println(string text):void @end
```

C 函数实现

```asm
; Native函数命需要按照'命名空间_函数名'形式定义
void System_println(RuntimeContext *context) {
    // 由于虚拟机内部字符串被设计为不可变，通过该方法获取到的是虚拟机字符串的副本,
    // 调用者具有该数据所有权需要显示释放防止内存泄漏.
    const KniString str =*pvm_kni_get_str(context,0);
    if (str == NULL) {
        printf("%s\n", "null");
        return;
    }
    printf("%s\n", str->buf);
    // 显示释放字符串
    pvm_kni_release_str(&str);
    
}

```

## 📜 许可证

PixtronVM采用 **Apache License 2.0** 开源协议分发

```
Copyright 2023 PixtronVM Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```