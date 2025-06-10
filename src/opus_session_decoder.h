#ifndef OPUS_SESSION_DECODER_H
#define OPUS_SESSION_DECODER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

// Forward declaration for Opus
struct OpusDecoder;

using namespace godot;

class OpusSessionDecoder : public RefCounted {
    GDCLASS(OpusSessionDecoder, RefCounted)

private:
    static constexpr int SAMPLE_RATE = 16000;  // Fixed sample rate at 16000Hz
    static constexpr int CHANNELS = 1;         // Mono channel
    static constexpr int MAX_FRAME_SIZE = SAMPLE_RATE * 120 / 1000;  // 120ms max frame size

    OpusDecoder* decoder;
    bool session_active;
    int64_t total_decoded_samples;
    int packet_count;

protected:
    static void _bind_methods();

public:
    OpusSessionDecoder();
    ~OpusSessionDecoder();

    // Session management
    bool start_session();                                           // 开始解码会话
    void end_session();                                            // 结束解码会话
    void reset_session();                                          // 重置会话状态
    bool is_session_active() const;                               // 检查会话状态

    // Decoding methods
    PackedByteArray decode_packet(const PackedByteArray& opus_data);  // 解码单个包
    PackedByteArray decode_packets(const Array& opus_packets);        // 批量解码包
    
    // Statistics and info
    int64_t get_total_decoded_samples() const;                     // 获取总解码样本数
    double get_total_decoded_duration() const;                     // 获取总解码时长
    int get_packet_count() const;                                  // 获取已处理包数量
    void reset_statistics();                                       // 重置统计信息
    
    // Audio parameters
    int get_sample_rate() const { return SAMPLE_RATE; }
    int get_channels() const { return CHANNELS; }
    int get_max_frame_size() const { return MAX_FRAME_SIZE; }
};

#endif // OPUS_SESSION_DECODER_H 