# Godot P3Opus Plugin

**English** | [中文版](README_CN.md)

A GDExtension plugin for Godot 4.4 that decodes P3 format audio data. P3 format is a custom format containing Opus-encoded audio data.

## Project Structure

```
Godot-P3Opus-Plugin/
├── src/                    # Source code directory
│   ├── register_types.cpp  # GDExtension registration code
│   ├── register_types.h
│   ├── p3_decoder.cpp      # P3 decoder implementation
│   └── p3_decoder.h        # P3 decoder header file
├── godot-cpp/              # Godot C++ bindings (submodule)
├── third/                  # Third-party libraries
│   └── opus/               # Opus audio library (submodule)
├── demo/                   # Example project
│   ├── project.godot       # Godot example project
│   ├── console.gd          # Example script
│   ├── voice.p3            # Example P3 audio file
│   ├── extension.gdextension # Godot extension configuration file
│   └── README_P3Decoder.md # Detailed P3 decoder usage guide
├── build/                  # Build output directory
├── CMakeLists.txt          # CMake configuration file
├── build.sh                # Build script
├── README.md               # Project documentation (English)
└── README_CN.md            # Project documentation (Chinese)
```

## Features

- **P3 Format Decoding**: Decode Opus audio data from P3 format binary data
- **Memory-based Processing**: Direct binary data processing without file I/O operations
- **PCM Output**: Returns 16-bit PCM data that can be used directly with AudioStreamWAV
- **Fixed Parameters**: Supports 16000Hz sample rate, mono audio
- **Cross-Platform**: Supports Windows, macOS, Linux
- **Easy Integration**: Works as a GDExtension plugin, can be used directly in Godot projects

## Requirements

- CMake 3.16 or higher
- Git
- C++17 compatible compiler
- Godot 4.1 or higher
- Supported platforms: Windows, macOS, Linux

## Quick Start

### 1. Clone the Project

```bash
git clone <your-repo-url>
cd Godot-P3Opus-Plugin
```

### 2. Initialize Submodules

```bash
git submodule update --init --recursive
```

Or use the build script:

```bash
./build.sh init
```

### 3. Build the Project

#### Using Build Script (Recommended)

```bash
# Build Release version
./build.sh build

# Build Debug version
./build.sh build debug

# Clean build files
./build.sh clean

# Show help
./build.sh help
```

#### Manual CMake Build

```bash
# Create build directory
mkdir build && cd build

# Configure project
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compile
cmake --build . --config Release -j4

# Return to project root
cd ..
```

## Build Output

After building, files will be generated at the following locations:

- **Shared Library**: `build/lib/libp3opus.[platform].template_[config].[arch].[ext]`
- **Extension Configuration**: `demo/extension.gdextension` (pre-configured)

Where:
- `platform`: windows/macos/linux
- `config`: debug/release
- `arch`: x86_64/universal, etc.
- `ext`: dll/dylib/so

## Using in Godot

### 1. Install Plugin

**Option 1: Use the demo project directly**
1. Open the `demo/` directory as a Godot project
2. The extension is already configured and ready to use

**Option 2: Install in your own project**
1. Copy the generated shared library files to your Godot project directory
2. Copy the `demo/extension.gdextension` file to your project root directory
3. Adjust the library paths in `extension.gdextension` if necessary
4. Reload the project in Godot editor

### 2. Basic Usage

```gdscript
extends Node

func _ready():
    # Create P3 decoder
    var decoder = P3Decoder.new()
    
    # Load P3 file as binary data
    var file = FileAccess.open("res://voice.p3", FileAccess.READ)
    if file == null:
        print("Failed to open P3 file")
        return
    
    var p3_data = file.get_buffer(file.get_length())
    file.close()
    
    # Decode P3 binary data
    var pcm_data = decoder.decode_p3(p3_data)
    
    if pcm_data.size() > 0:
        # Create audio stream
        var stream = AudioStreamWAV.new()
        stream.format = AudioStreamWAV.FORMAT_16_BITS
        stream.mix_rate = decoder.get_sample_rate()  # 16000Hz
        stream.stereo = false  # Mono
        stream.data = pcm_data
        
        # Play audio
        var player = AudioStreamPlayer.new()
        player.stream = stream
        add_child(player)
        player.play()
    else:
        print("P3 data decoding failed")

# Alternative usage with network data
func decode_network_p3_data(p3_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_p3(p3_bytes)
    
    if pcm_data.size() > 0:
        print("Successfully decoded P3 data from network")
        # Process PCM data...
    else:
        print("Failed to decode network P3 data")
```

### 3. Run Example

The project includes a complete example in the `demo/` directory:

1. Open Godot and import the `demo/` directory as a project
2. The extension is pre-configured with correct library paths
3. Run the main scene to see the P3 decoder in action
4. Check the console output for decoding results

## P3 File Format

P3 files consist of multiple data packets, each with the following structure:

```
+------------------+
| packet_type (1B) |  // Packet type
+------------------+
| reserved (1B)    |  // Reserved field
+------------------+
| data_len (2B)    |  // Data length (big-endian)
+------------------+
| opus_data (...)  |  // Opus encoded data
+------------------+
```

## API Reference

### P3Decoder Class

#### Methods

- `decode_p3(p3_data: PackedByteArray) -> PackedByteArray`
  - Decodes P3 format binary data and returns 16-bit PCM data
  - Parameter: P3 binary data as PackedByteArray
  - Returns: PCM audio data, empty array on failure
  - Use cases: File data, network streams, memory buffers

- `get_sample_rate() -> int`
  - Gets the audio sample rate (fixed at 16000Hz)

- `get_channels() -> int`
  - Gets the number of audio channels (fixed at 1, mono)

#### Usage Examples

**Decode from file:**
```gdscript
var file = FileAccess.open("res://audio.p3", FileAccess.READ)
var p3_data = file.get_buffer(file.get_length())
file.close()

var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(p3_data)
```

**Decode from network:**
```gdscript
# Assuming you received p3_bytes from network
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(p3_bytes)
```

**Decode from memory buffer:**
```gdscript
# Assuming you have P3 data in memory
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(your_p3_buffer)
```

For detailed API documentation, see: `demo/README_P3Decoder.md`

## Troubleshooting

### Common Issues

1. **CMake Cache Error**
   ```bash
   # If you encounter CMakeCache.txt directory errors, clean the build directory
   rm -rf build/
   mkdir build
   ```

2. **Submodules Not Initialized**
   ```bash
   git submodule update --init --recursive
   ```

3. **CMake Version Too Old**
   - Please upgrade to CMake 3.16 or higher

4. **Compilation Errors**
   - Ensure you have a C++17 compatible compiler installed
   - Check that all submodules are properly initialized

5. **Cannot Load Extension in Godot**
   - Ensure the extension.gdextension file is in the project root directory
   - Check that the shared library file paths in extension.gdextension are correct
   - For the demo project, the paths are pre-configured as `../build/lib/...`
   - For your own project, adjust the paths to point to the correct library location
   - Confirm Godot version compatibility (requires 4.1+)

6. **Empty PCM Data Returned**
   - Check if the input P3 data is valid and not empty
   - Verify the P3 data format matches the expected structure
   - Check console output for specific error messages

### Getting Help

If you encounter issues, please:

1. Check the error messages in the build logs
2. Confirm all dependencies are properly installed
3. Try cleaning the build directory and rebuilding
4. Check the example usage in the demo directory
5. Verify your P3 data format is correct

## License

Please refer to the license files of the related libraries:
- Opus: See `third/opus/COPYING`
- Godot-cpp: See `godot-cpp/LICENSE` 