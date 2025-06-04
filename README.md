# OpusLib GDExtension

这是一个集成了Opus音频编解码库的Godot 4.4 GDExtension项目。

## 项目结构

```
OpusLib/
├── src/                    # 源代码目录
│   ├── register_types.cpp  # GDExtension注册代码
│   ├── register_types.h
│   ├── debuger.cpp         # 示例类
│   └── debuger.h
├── godot-cpp/              # Godot C++绑定 (子模块)
├── third/                  # 第三方库
│   └── opus/               # Opus音频库 (子模块)
├── CMakeLists.txt          # CMake配置文件
├── extension.gdextension.in # Godot扩展配置模板
├── build.sh                # 构建脚本
└── README.md               # 项目说明
```

## 依赖要求

- CMake 3.16 或更高版本
- Git
- C++17 兼容的编译器
- 支持的平台：Windows、macOS、Linux

## 快速开始

### 1. 克隆项目

```bash
git clone <your-repo-url>
cd OpusLib
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

- **共享库**: `build/lib/libopuslib.[platform].[config].[arch].[ext]`
- **扩展配置**: `build/extension.gdextension`

其中：
- `platform`: windows/macos/linux
- `config`: template_debug/template_release
- `arch`: x86_64/universal/arm64等
- `ext`: dll/dylib/so

## 在Godot中使用

1. 将生成的共享库文件复制到你的Godot项目目录
2. 将`extension.gdextension`文件复制到项目根目录
3. 在Godot编辑器中重新加载项目

## CMake配置说明

### 主要特性

- **跨平台支持**: 自动检测平台并设置相应的构建参数
- **Opus集成**: 自动构建和链接Opus音频库
- **Godot-cpp集成**: 自动构建和链接Godot C++绑定
- **优化设置**: 针对不同构建类型的编译器优化
- **符号可见性**: 正确设置共享库的符号可见性

### 自定义选项

你可以通过CMake变量来自定义构建：

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DOPUS_BUILD_SHARED_LIBRARY=ON \
         -DOPUS_BUILD_TESTING=OFF
```

### 支持的CMake变量

- `CMAKE_BUILD_TYPE`: 构建类型 (Debug/Release)
- `OPUS_BUILD_SHARED_LIBRARY`: 构建Opus共享库
- `OPUS_BUILD_TESTING`: 构建Opus测试
- `OPUS_BUILD_PROGRAMS`: 构建Opus程序

## 开发说明

### 添加新的源文件

CMake配置会自动扫描`src/`目录下的所有`.cpp`和`.c`文件，所以你只需要将新文件放在该目录下即可。

### 修改扩展配置

如果需要修改Godot扩展的配置，请编辑`extension.gdextension.in`文件。

### 添加新的依赖

如果需要添加新的第三方库，请：

1. 将库添加到`third/`目录
2. 在`CMakeLists.txt`中添加相应的`add_subdirectory()`调用
3. 在`target_link_libraries()`中添加库的链接

## 故障排除

### 常见问题

1. **子模块未初始化**
   ```bash
   git submodule update --init --recursive
   ```

2. **CMake版本过低**
   - 请升级到CMake 3.16或更高版本

3. **编译错误**
   - 确保安装了C++17兼容的编译器
   - 检查是否正确初始化了所有子模块

4. **链接错误**
   - 确保Opus库正确构建
   - 检查目标平台设置是否正确

### 获取帮助

如果遇到问题，请：

1. 检查构建日志中的错误信息
2. 确认所有依赖都已正确安装
3. 尝试清理构建目录后重新构建

## 许可证

请查看相关库的许可证文件：
- Opus: 参见 `third/opus/COPYING`
- Godot-cpp: 参见 `godot-cpp/LICENSE` 