# Godot P3Opus Plugin

**English** | [中文版](README_CN.md)

A GDExtension plugin for Godot 4.4 that decodes P3 format and pure Opus audio data. P3 format is a custom format containing Opus-encoded audio data.

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

- **P3 Format Decoding**: Decode Opus audio data from P3 format binary data (multi-packet)
- **Pure Opus Decoding**: Decode raw Opus binary data (single packet)
- **Memory-based Processing**: Direct binary data processing without file I/O operations
- **PCM Output**: Returns 16-bit PCM data that can be used directly with AudioStreamWAV
- **Fixed Parameters**: Supports 16000Hz sample rate, mono audio
- **Cross-Platform**: Supports Windows, macOS, Linux
- **Easy Integration**: Works as a GDExtension plugin, can be used directly in Godot projects
- **Modular Architecture**: Shared core decoding logic for both P3 and Opus formats

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

#### Decoding P3 Format Data

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
    
    # Decode P3 binary data (multi-packet format)
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
```

#### Decoding Pure Opus Data

```gdscript
extends Node

func decode_opus_packet(opus_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    
    # Decode pure Opus binary data (single packet)
    var pcm_data = decoder.decode_opus(opus_bytes)
    
    if pcm_data.size() > 0:
        print("Successfully decoded Opus packet")
        
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
        print("Opus data decoding failed")
```

#### Network Data Processing

```gdscript
# Process P3 data from network
func decode_network_p3_data(p3_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_p3(p3_bytes)
    
    if pcm_data.size() > 0:
        print("Successfully decoded P3 data from network")
        # Process PCM data...
    else:
        print("Failed to decode network P3 data")

# Process Opus data from network
func decode_network_opus_data(opus_bytes: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_opus(opus_bytes)
    
    if pcm_data.size() > 0:
        print("Successfully decoded Opus data from network")
        # Process PCM data...
    else:
        print("Failed to decode network Opus data")
```

### 3. Run Example

The project includes a complete example in the `demo/` directory:

1. Open Godot and import the `demo/` directory as a project
2. The extension is pre-configured with correct library paths
3. Run the main scene to see the P3 decoder in action
4. Check the console output for decoding results

## Data Format Specifications

### P3 File Format

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

### Pure Opus Format

Pure Opus data is raw Opus-encoded binary data without any wrapper headers. This is typically used for:
- Network streaming
- Real-time audio transmission
- Direct Opus packet processing

## API Reference

### P3Decoder Class

#### Methods

- **`decode_p3(p3_data: PackedByteArray) -> PackedByteArray`**
  - Decodes P3 format binary data and returns 16-bit PCM data
  - Parameter: P3 binary data as PackedByteArray (multi-packet format)
  - Returns: PCM audio data, empty array on failure
  - Use cases: P3 files, P3 network streams, P3 memory buffers

- **`decode_opus(opus_data: PackedByteArray) -> PackedByteArray`**
  - Decodes pure Opus binary data and returns 16-bit PCM data
  - Parameter: Pure Opus binary data as PackedByteArray (single packet)
  - Returns: PCM audio data, empty array on failure
  - Use cases: Raw Opus packets, network streaming, real-time audio

- **`get_sample_rate() -> int`**
  - Gets the audio sample rate (fixed at 16000Hz)
  - Returns: 16000

- **`get_channels() -> int`**
  - Gets the number of audio channels (fixed at 1, mono)
  - Returns: 1

#### Usage Examples

**Decode P3 from file:**
```gdscript
var file = FileAccess.open("res://audio.p3", FileAccess.READ)
var p3_data = file.get_buffer(file.get_length())
file.close()

var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(p3_data)
```

**Decode Opus from network:**
```gdscript
# Assuming you received opus_bytes from network
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_opus(opus_bytes)
```

**Decode P3 from memory buffer:**
```gdscript
# Assuming you have P3 data in memory
var decoder = P3Decoder.new()
var pcm_data = decoder.decode_p3(your_p3_buffer)
```

**Real-time Opus processing:**
```gdscript
# Process individual Opus packets in real-time
func process_opus_packet(packet: PackedByteArray):
    var decoder = P3Decoder.new()
    var pcm_data = decoder.decode_opus(packet)
    
    if pcm_data.size() > 0:
        # Immediately process or play the decoded audio
        play_pcm_data(pcm_data)
```

For detailed API documentation, see: `demo/README_P3Decoder.md`

## Architecture

The plugin uses a modular architecture with shared core decoding logic:

- **`decode_p3()`**: Parses P3 headers and processes multiple Opus packets
- **`decode_opus()`**: Directly processes single Opus packets
- **`decode_opus_packet()`**: Private helper method containing the core Opus decoding logic

This design ensures code reuse and maintainability while providing flexible interfaces for different use cases.

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
   - Check if the input data is valid and not empty
   - For P3 data: Verify the P3 data format matches the expected structure
   - For Opus data: Ensure the data is valid Opus-encoded audio
   - Check console output for specific error messages

7. **Audio Quality Issues**
   - Ensure the input data matches the expected format (16000Hz, mono)
   - Check that the Opus data was encoded with compatible parameters
   - Verify the PCM output is being used correctly in AudioStreamWAV

### Getting Help

If you encounter issues, please:

1. Check the error messages in the build logs
2. Confirm all dependencies are properly installed
3. Try cleaning the build directory and rebuilding
4. Check the example usage in the demo directory
5. Verify your data format is correct (P3 vs pure Opus)
6. Test with the provided example files first

## Performance Notes

- Both `decode_p3()` and `decode_opus()` are optimized for real-time processing
- Memory allocation is minimized during decoding
- The plugin is suitable for both batch processing and streaming applications
- For high-frequency calls, consider reusing the P3Decoder instance

## License

Please refer to the license files of the related libraries:
- Opus: See `third/opus/COPYING`
- Godot-cpp: See `godot-cpp/LICENSE` 