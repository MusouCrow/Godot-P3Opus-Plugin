# Godot P3Opus 插件

[English](README.md) | **中文版**

一个用于 Godot 4.4 的 GDExtension 插件，可以解码 P3 格式和纯 Opus 音频数据。P3 格式是包含 Opus 编码音频数据的自定义格式。

## 项目结构

```
Godot-P3Opus-Plugin/
├── src/                    # 源代码目录
│   ├── register_types.cpp  # GDExtension 注册代码
│   ├── register_types.h
│   ├── p3_decoder.cpp      # P3 解码器实现
│   └── p3_decoder.h        # P3 解码器头文件
├── godot-cpp/              # Godot C++ 绑定（子模块）
├── third/                  # 第三方库
│   └── opus/               # Opus 音频库（子模块）
├── demo/                   # 示例项目
│   ├── project.godot       # Godot 示例项目
│   ├── console.gd          # 示例脚本
│   ├── voice.p3            # 示例 P3 音频文件
│   ├── extension.gdextension # Godot 扩展配置文件
│   └── README_P3Decoder.md # 详细的 P3 解码器使用指南
├── build/                  # 构建输出目录
├── CMakeLists.txt          # CMake 配置文件
├── build.sh                # 构建脚本
├── README.md               # 项目文档（英文）
└── README_CN.md            # 项目文档（中文）
```

## 功能特性

- **P3 格式解码**：解码 P3 格式二进制数据中的 Opus 音频数据（多包格式）
- **纯 Opus 解码**：解码原始 Opus 二进制数据（单包格式）
- **内存处理**：直接处理二进制数据，无需文件 I/O 操作
- **PCM 输出**：返回 16 位 PCM 数据，可直接用于 AudioStreamWAV
- **固定参数**：支持 16000Hz 采样率，单声道音频
- **跨平台**：支持 Windows、macOS、Linux
- **易于集成**：作为 GDExtension 插件工作，可直接在 Godot 项目中使用
- **模块化架构**：P3 和 Opus 格式共享核心解码逻辑

## 系统要求

- CMake 3.16 或更高版本
- Git
- 支持 C++17 的编译器
- Godot 4.1 或更高版本
- 支持的平台：Windows、macOS、Linux

## 快速开始

### 1. 克隆项目

```bash
git clone <your-repo-url>
cd Godot-P3Opus-Plugin
```

### 2. 初始化子模块

```bash
git submodule update --init --recursive
```

或使用构建脚本：

```bash
./build.sh init
```

### 3. 构建项目

#### 使用构建脚本（推荐）

```bash
# 构建 Release 版本
./build.sh build

# 构建 Debug 版本
./build.sh build debug

# 清理构建文件
./build.sh clean

# 显示帮助
./build.sh help
```

#### 手动 CMake 构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release -j4

# 返回项目根目录
cd ..
```

## 构建输出

构建完成后，文件将生成在以下位置：

- **共享库**：`build/lib/libp3opus.[platform].template_[config].[arch].[ext]`
- **扩展配置**：`demo/extension.gdextension`（已预配置）

其中：
- `platform`：windows/macos/linux
- `config`：debug/release
- `arch`：x86_64/universal 等
- `ext`：dll/dylib/so

## 在 Godot 中使用

### 1. 安装插件

**方式一：直接使用演示项目**
1. 在 Godot 中打开 `demo/` 目录作为项目
2. 扩展已配置完成，可直接使用

**方式二：安装到自己的项目**
1. 将生成的共享库文件复制到你的 Godot 项目目录
2. 将 `demo/extension.gdextension` 文件复制到项目根目录
3. 如有必要，调整 `extension.gdextension` 中的库路径
4. 在 Godot 编辑器中重新加载项目

### 2. 基本用法

#### 解码 P3 格式数据

```gdscript
extends Node

func _ready():
    # 创建 P3 解码器
    var decoder = P3Decoder.new()
    
    # 加载 P3 文件为二进制数据
    var file = FileAccess.open("res://voice.p3", FileAccess.READ)
    if file == null:
        print("无法打开 P3 文件")
        return
    
    var p3_data = file.get_buffer(file.get_length())
    file.close()
    
    # 解码 P3 二进制数据（多包格式）
    var pcm_data = decoder.decode_p3(p3_data)
    
    if pcm_data.size() > 0:
        # 创建音频流
        var stream = AudioStreamWAV.new()
        stream.format = AudioStreamWAV.FORMAT_16_BITS
        stream.mix_rate = decoder.get_sample_rate()  # 16000Hz
        stream.stereo = false  # 单声道
        stream.data = pcm_data
        
        # 播放音频
        var player = AudioStreamPlayer.new()
        player.stream = stream
        add_child(player)
        player.play()
    else:
        print("P3 数据解码失败")
```

#### 解码纯 Opus 数据

```gdscript
extends Node

func decode_opus_packet(opus_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    
    # 解码纯 Opus 二进制数据（单包格式）
    var pcm_data = decoder.decode_opus(opus_bytes)
    
    if pcm_data.size() > 0:
        print("成功解码 Opus 数据包")
        
        # 创建音频流
        var stream = AudioStreamWAV.new()
        stream.format = AudioStreamWAV.FORMAT_16_BITS
        stream.mix_rate = decoder.get_sample_rate()  # 16000Hz
        stream.stereo = false  # 单声道
        stream.data = pcm_data
        
        # 播放音频
        var player = AudioStreamPlayer.new()
        player.stream = stream
        add_child(player)
        player.play()
    else:
        print("Opus 数据解码失败")
```

#### 网络数据处理

```gdscript
# 处理来自网络的 P3 数据
func decode_network_p3_data(p3_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_p3(p3_bytes)
    
    if pcm_data.size() > 0:
        print("成功解码网络 P3 数据")
        # 处理 PCM 数据...
    else:
        print("网络 P3 数据解码失败")

# 处理来自网络的 Opus 数据
func decode_network_opus_data(opus_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_opus(opus_bytes)
    
    if pcm_data.size() > 0:
        print("成功解码网络 Opus 数据")
        # 处理 PCM 数据...
    else:
        print("网络 Opus 数据解码失败")
```

### 3. 运行示例

项目在 `demo/` 目录中包含完整示例：

1. 在 Godot 中导入 `demo/` 目录作为项目
2. 扩展已预配置正确的库路径
3. 运行主场景查看 P3 解码器的运行效果
4. 查看控制台输出了解解码结果

## 数据格式规范

### P3 文件格式

P3 文件由多个数据包组成，每个包具有以下结构：

```
+------------------+
| packet_type (1B) |  // 包类型
+------------------+
| reserved (1B)    |  // 保留字段
+------------------+
| data_len (2B)    |  // 数据长度（大端序）
+------------------+
| opus_data (...)  |  // Opus 编码数据
+------------------+
```

### 纯 Opus 格式

纯 Opus 数据是没有任何包装头的原始 Opus 编码二进制数据。通常用于：
- 网络流传输
- 实时音频传输
- 直接 Opus 包处理

## API 参考

### P3Decoder 类

#### 方法

- **`decode_p3(p3_data: PackedByteArray) -> PackedByteArray`**
  - 解码 P3 格式二进制数据并返回 16 位 PCM 数据
  - 参数：P3 二进制数据（PackedByteArray，多包格式）
  - 返回：PCM 音频数据，失败时返回空数组
  - 使用场景：P3 文件、P3 网络流、P3 内存缓冲区

- **`decode_opus(opus_data: PackedByteArray) -> PackedByteArray`**
  - 解码纯 Opus 二进制数据并返回 16 位 PCM 数据
  - 参数：纯 Opus 二进制数据（PackedByteArray，单包格式）
  - 返回：PCM 音频数据，失败时返回空数组
  - 使用场景：原始 Opus 包、网络流、实时音频

- **`get_sample_rate() -> int`**
  - 获取音频采样率（固定为 16000Hz）
  - 返回：16000

- **`get_channels() -> int`**
  - 获取音频声道数（固定为 1，单声道）
  - 返回：1

#### 使用示例

**从文件解码 P3：**
```gdscript
var file = FileAccess.open("res://audio.p3", FileAccess.READ)
var p3_data = file.get_buffer(file.get_length())
file.close()

var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(p3_data)
```

**从网络解码 Opus：**
```gdscript
# 假设你从网络接收到 opus_bytes
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_opus(opus_bytes)
```

**从内存缓冲区解码 P3：**
```gdscript
# 假设你在内存中有 P3 数据
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(your_p3_buffer)
```

**实时 Opus 处理：**
```gdscript
# 实时处理单个 Opus 包
func process_opus_packet(packet: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_opus(packet)
    
    if pcm_data.size() > 0:
        # 立即处理或播放解码的音频
        play_pcm_data(pcm_data)
```

详细的 API 文档请参见：`demo/README_P3Decoder.md`

## 架构设计

插件采用模块化架构，共享核心解码逻辑：

- **`decode_p3()`**：解析 P3 头部并处理多个 Opus 包
- **`decode_opus()`**：直接处理单个 Opus 包
- **`decode_opus_packet()`**：包含核心 Opus 解码逻辑的私有辅助方法

这种设计确保了代码重用和可维护性，同时为不同使用场景提供了灵活的接口。

## 故障排除

### 常见问题

1. **CMake 缓存错误**
   ```bash
   # 如果遇到 CMakeCache.txt 目录错误，清理构建目录
   rm -rf build/
   mkdir build
   ```

2. **子模块未初始化**
   ```bash
   git submodule update --init --recursive
   ```

3. **CMake 版本过旧**
   - 请升级到 CMake 3.16 或更高版本

4. **编译错误**
   - 确保安装了支持 C++17 的编译器
   - 检查所有子模块是否正确初始化

5. **无法在 Godot 中加载扩展**
   - 确保 extension.gdextension 文件在项目根目录
   - 检查 extension.gdextension 中的共享库文件路径是否正确
   - 对于演示项目，路径已预配置为 `../build/lib/...`
   - 对于自己的项目，调整路径指向正确的库位置
   - 确认 Godot 版本兼容性（需要 4.1+）

6. **返回空的 PCM 数据**
   - 检查输入数据是否有效且非空
   - 对于 P3 数据：验证 P3 数据格式是否符合预期结构
   - 对于 Opus 数据：确保数据是有效的 Opus 编码音频
   - 查看控制台输出获取具体错误信息

7. **音频质量问题**
   - 确保输入数据符合预期格式（16000Hz，单声道）
   - 检查 Opus 数据是否使用兼容参数编码
   - 验证 PCM 输出在 AudioStreamWAV 中的使用是否正确

### 获取帮助

如果遇到问题，请：

1. 检查构建日志中的错误信息
2. 确认所有依赖项已正确安装
3. 尝试清理构建目录并重新构建
4. 查看演示目录中的示例用法
5. 验证数据格式是否正确（P3 vs 纯 Opus）
6. 首先使用提供的示例文件进行测试

## 性能说明

- `decode_p3()` 和 `decode_opus()` 都针对实时处理进行了优化
- 解码过程中内存分配最小化
- 插件适用于批处理和流式应用
- 对于高频调用，建议重用 P3Decoder 实例

## 许可证

请参考相关库的许可证文件：
- Opus：参见 `third/opus/COPYING`
- Godot-cpp：参见 `godot-cpp/LICENSE` 