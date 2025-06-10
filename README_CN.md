# Godot P3Opus Plugin

[English](README.md) | **中文版**

这是一个为Godot 4.4开发的GDExtension插件，用于解码P3格式的音频数据。P3格式是一种包含Opus编码音频数据的自定义格式。

## 项目结构

```
Godot-P3Opus-Plugin/
├── src/                    # 源代码目录
│   ├── register_types.cpp  # GDExtension注册代码
│   ├── register_types.h
│   ├── p3_decoder.cpp      # P3解码器实现
│   └── p3_decoder.h        # P3解码器头文件
├── godot-cpp/              # Godot C++绑定 (子模块)
├── third/                  # 第三方库
│   └── opus/               # Opus音频库 (子模块)
├── demo/                   # 示例项目
│   ├── project.godot       # Godot示例项目
│   ├── console.gd          # 示例脚本
│   ├── voice.p3            # 示例P3音频文件
│   ├── extension.gdextension # Godot扩展配置文件
│   └── README_P3Decoder.md # P3解码器详细使用说明
├── build/                  # 构建输出目录
├── CMakeLists.txt          # CMake配置文件
├── build.sh                # 构建脚本
├── README.md               # 项目说明（英文版）
└── README_CN.md            # 项目说明（中文版）
```

## 功能特性

- **P3格式解码**: 解码P3格式二进制数据中的Opus音频数据
- **内存处理**: 直接处理二进制数据，无需文件I/O操作
- **PCM输出**: 返回16位PCM数据，可直接用于AudioStreamWAV
- **固定参数**: 支持16000Hz采样率，单声道音频
- **跨平台**: 支持Windows、macOS、Linux
- **易于集成**: 作为GDExtension插件，可直接在Godot项目中使用

## 依赖要求

- CMake 3.16 或更高版本
- Git
- C++17 兼容的编译器
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

或者使用构建脚本：

```bash
./build.sh init
```

### 3. 构建项目

#### 使用构建脚本（推荐）

```bash
# 构建Release版本
./build.sh build

# 构建Debug版本
./build.sh build debug

# 清理构建文件
./build.sh clean

# 显示帮助
./build.sh help
```

#### 手动使用CMake

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

构建完成后，会在以下位置生成文件：

- **共享库**: `build/lib/libp3opus.[platform].template_[config].[arch].[ext]`
- **扩展配置**: `demo/extension.gdextension`（已预配置）

其中：
- `platform`: windows/macos/linux
- `config`: debug/release
- `arch`: x86_64/universal等
- `ext`: dll/dylib/so

## 在Godot中使用

### 1. 安装插件

**方式一：直接使用示例项目**
1. 在Godot中打开`demo/`目录作为项目
2. 扩展已经配置好，可以直接使用

**方式二：安装到自己的项目**
1. 将构建生成的共享库文件复制到你的Godot项目目录
2. 将`demo/extension.gdextension`文件复制到项目根目录
3. 如有必要，调整`extension.gdextension`中的库文件路径
4. 在Godot编辑器中重新加载项目

### 2. 基本用法

```gdscript
extends Node

func _ready():
    # 创建P3解码器
    var decoder = P3Decoder.new()
    
    # 加载P3文件为二进制数据
    var file = FileAccess.open("res://voice.p3", FileAccess.READ)
    if file == null:
        print("无法打开P3文件")
        return
    
    var p3_data = file.get_buffer(file.get_length())
    file.close()
    
    # 解码P3二进制数据
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
        print("P3数据解码失败")

# 网络数据解码的替代用法
func decode_network_p3_data(p3_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_p3(p3_bytes)
    
    if pcm_data.size() > 0:
        print("成功解码来自网络的P3数据")
        # 处理PCM数据...
    else:
        print("网络P3数据解码失败")
```

### 3. 运行示例

项目在`demo/`目录中包含一个完整的示例：

1. 在Godot中导入`demo/`目录作为项目
2. 扩展配置文件已预设正确的库文件路径
3. 运行主场景查看P3解码器的使用效果
4. 查看控制台输出了解解码结果

## P3文件格式

P3文件由多个数据包组成，每个数据包的结构如下：

```
+------------------+
| packet_type (1B) |  // 数据包类型
+------------------+
| reserved (1B)    |  // 保留字段
+------------------+
| data_len (2B)    |  // 数据长度(大端序)
+------------------+
| opus_data (...)  |  // Opus编码数据
+------------------+
```

## API参考

### P3Decoder类

#### 方法

- `decode_p3(p3_data: PackedByteArray) -> PackedByteArray`
  - 解码P3格式二进制数据，返回16位PCM数据
  - 参数：P3二进制数据（PackedByteArray类型）
  - 返回：PCM音频数据，失败时返回空数组
  - 使用场景：文件数据、网络流、内存缓冲区

- `get_sample_rate() -> int`
  - 获取音频采样率（固定为16000Hz）

- `get_channels() -> int`
  - 获取音频声道数（固定为1，单声道）

#### 使用示例

**从文件解码：**
```gdscript
var file = FileAccess.open("res://audio.p3", FileAccess.READ)
var p3_data = file.get_buffer(file.get_length())
file.close()

var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(p3_data)
```

**从网络解码：**
```gdscript
# 假设你从网络接收到了p3_bytes
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(p3_bytes)
```

**从内存缓冲区解码：**
```gdscript
# 假设你在内存中有P3数据
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(your_p3_buffer)
```

详细的API文档请参考：`demo/README_P3Decoder.md`

## 故障排除

### 常见问题

1. **CMake缓存错误**
   ```bash
   # 如果遇到CMakeCache.txt目录错误，清理构建目录
   rm -rf build/
   mkdir build
   ```

2. **子模块未初始化**
   ```bash
   git submodule update --init --recursive
   ```

3. **CMake版本过旧**
   - 请升级到CMake 3.16或更高版本

4. **编译错误**
   - 确保安装了C++17兼容的编译器
   - 检查所有子模块是否正确初始化

5. **Godot中无法加载扩展**
   - 确保extension.gdextension文件在项目根目录
   - 检查extension.gdextension中的共享库文件路径是否正确
   - 对于示例项目，路径已预配置为`../build/lib/...`
   - 对于自己的项目，调整路径指向正确的库文件位置
   - 确认Godot版本兼容性（需要4.1+）

6. **返回空的PCM数据**
   - 检查输入的P3数据是否有效且不为空
   - 验证P3数据格式是否符合预期结构
   - 查看控制台输出获取具体错误信息

### 获取帮助

如果遇到问题，请：

1. 检查构建日志中的错误信息
2. 确认所有依赖项都已正确安装
3. 尝试清理构建目录并重新构建
4. 查看demo目录中的示例用法
5. 验证你的P3数据格式是否正确

## 许可证

请参考相关库的许可证文件：
- Opus: 参见 `third/opus/COPYING`
- Godot-cpp: 参见 `godot-cpp/LICENSE` 