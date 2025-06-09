#ifndef P3_DECODER_H
#define P3_DECODER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

class P3Decoder : public RefCounted {
    GDCLASS(P3Decoder, RefCounted)

private:
    // P3 format header structure
    struct P3Header {
        uint8_t packet_type;    // 1 byte type
        uint8_t reserved;       // 1 byte reserved
        uint16_t data_len;      // 2 bytes length (big endian)
    };

    static constexpr int SAMPLE_RATE = 16000;  // Fixed sample rate at 16000Hz
    static constexpr int CHANNELS = 1;         // Mono channel
    static constexpr int FRAME_SIZE = SAMPLE_RATE * 60 / 1000;  // 60ms frame size

    // Convert from big endian to host byte order
    uint16_t be16_to_host(uint16_t value);

protected:
    static void _bind_methods();

public:
    P3Decoder();
    ~P3Decoder();

    // Decode P3 binary data and return PCM data
    PackedByteArray decode_p3(const PackedByteArray& p3_data);
    
    // Get audio parameters
    int get_sample_rate() const { return SAMPLE_RATE; }
    int get_channels() const { return CHANNELS; }
};

#endif // P3_DECODER_H 