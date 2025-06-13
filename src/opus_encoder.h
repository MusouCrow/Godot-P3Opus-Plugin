#ifndef OPUS_ENCODER_H
#define OPUS_ENCODER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <opus.h>

using namespace godot;

class OpusEncoder : public RefCounted {
    GDCLASS(OpusEncoder, RefCounted)

private:
    ::OpusEncoder* encoder;  // Use :: to avoid name conflict
    
    static constexpr int SAMPLE_RATE = 16000;  // Fixed sample rate at 16000Hz
    static constexpr int CHANNELS = 1;         // Mono channel
    static constexpr int FRAME_SIZE = SAMPLE_RATE * 60 / 1000;  // 60ms frame size
    static constexpr int MAX_PACKET_SIZE = 4000;  // Maximum Opus packet size

protected:
    static void _bind_methods();

public:
    OpusEncoder();
    ~OpusEncoder();

    // Initialize encoder with bitrate
    bool initialize(int bitrate = 64000);
    
    // Encode PCM data to Opus format
    PackedByteArray encode(const PackedByteArray& pcm_data);
    
    // Set encoder parameters
    bool set_bitrate(int bitrate);
    bool set_complexity(int complexity);  // 0-10, higher = better quality but slower
    bool set_signal_type(int signal_type);  // OPUS_SIGNAL_VOICE or OPUS_SIGNAL_MUSIC
    
    // Get audio parameters
    int get_sample_rate() const { return SAMPLE_RATE; }
    int get_channels() const { return CHANNELS; }
    int get_frame_size() const { return FRAME_SIZE; }
    
    // Check if encoder is initialized
    bool is_initialized() const { return encoder != nullptr; }
    
    // Reset encoder state
    void reset();
};

#endif // OPUS_ENCODER_H 