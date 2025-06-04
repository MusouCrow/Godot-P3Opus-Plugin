# P3解码器使用说明

## 概述

P3Decoder是一个Godot扩展类，用于解码P3格式的音频文件。P3格式是一种包含Opus编码音频数据的自定义格式。

## 功能特性

- 解码P3格式文件中的Opus音频数据
- 返回16位PCM数据，可直接用于AudioStreamWAV
- 支持16000Hz采样率，单声道音频
- 自动处理大端序字节序转换
- 提供详细的解码进度和错误信息

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

## 使用方法

### 基本用法

```gdscript
extends Node

func _ready():
    # 创建解码器
    var decoder = P3Decoder.new()
    
    # 解码P3文件
    var pcm_data = decoder.decode_p3_file("res://assets/voice.p3")
    
    if pcm_data.size() > 0:
        # 创建音频流
        var stream = AudioStreamWAV.new()
        stream.format = AudioStreamWAV.FORMAT_16_BITS
        stream.mix_rate = decoder.get_sample_rate()  # 16000
        stream.stereo = false  # 单声道
        stream.data = pcm_data
        
        # 播放音频
        var player = AudioStreamPlayer.new()
        player.stream = stream
        add_child(player)
        player.play()
```

### 获取音频参数

```gdscript
var decoder = P3Decoder.new()
print("采样率: ", decoder.get_sample_rate())  # 16000
print("声道数: ", decoder.get_channels())     # 1
```

### 错误处理

```gdscript
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3_file("res://assets/voice.p3")

if pcm_data.size() == 0:
    print("解码失败: 文件不存在、格式错误或解码出错")
    return
```

## API参考

### 方法

#### `decode_p3_file(file_path: String) -> PackedByteArray`
解码指定的P3文件，返回PCM数据。

**参数:**
- `file_path`: P3文件的路径

**返回值:**
- `PackedByteArray`: 16位PCM音频数据，如果解码失败则返回空数组

#### `get_sample_rate() -> int`
获取音频采样率。

**返回值:**
- `int`: 采样率 (16000 Hz)

#### `get_channels() -> int`
获取音频声道数。

**返回值:**
- `int`: 声道数 (1 = 单声道)

## 注意事项

1. **文件格式**: 只支持.p3扩展名的文件
2. **音频参数**: 固定为16000Hz采样率，单声道
3. **内存使用**: 解码时会将整个PCM数据加载到内存中
4. **错误处理**: 解码失败时会在控制台输出详细错误信息
5. **依赖库**: 需要链接Opus库

## 示例文件

- `p3_test.gd`: 简单的P3文件播放示例
- `p3_player_example.gd`: 完整的P3播放器示例，包含错误处理和批量处理功能

## 构建要求

确保CMakeLists.txt中包含了Opus库的链接：

```cmake
target_link_libraries(${PROJECT_NAME} PRIVATE
    godot-cpp
    opus
)
``` 