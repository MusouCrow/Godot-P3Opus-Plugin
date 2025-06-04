#ifndef P3_DECODER_H
#define P3_DECODER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

class P3Decoder : public RefCounted {
    GDCLASS(P3Decoder, RefCounted)

private:
    // P3格式头部结构
    struct P3Header {
        uint8_t packet_type;    // 1字节类型
        uint8_t reserved;       // 1字节保留
        uint16_t data_len;      // 2字节长度 (大端序)
    };

    static constexpr int SAMPLE_RATE = 16000;  // 采样率固定为16000Hz
    static constexpr int CHANNELS = 1;         // 单声道
    static constexpr int FRAME_SIZE = SAMPLE_RATE * 60 / 1000;  // 60ms帧大小

    // 从大端序转换为主机字节序
    uint16_t be16_to_host(uint16_t value);
    
    // 检查文件扩展名是否为.p3
    bool is_p3_file(const String& filename);

protected:
    static void _bind_methods();

public:
    P3Decoder();
    ~P3Decoder();

    // 解码P3文件，返回PCM数据
    PackedByteArray decode_p3_file(const String& file_path);
    
    // 获取音频参数
    int get_sample_rate() const { return SAMPLE_RATE; }
    int get_channels() const { return CHANNELS; }
};

#endif // P3_DECODER_H 