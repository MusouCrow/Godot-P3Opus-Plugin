#include "opus_encoder.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

OpusEncoder::OpusEncoder() : encoder(nullptr) {
}

OpusEncoder::~OpusEncoder() {
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
}

void OpusEncoder::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "bitrate"), &OpusEncoder::initialize, DEFVAL(64000));
    ClassDB::bind_method(D_METHOD("encode", "pcm_data"), &OpusEncoder::encode);
    ClassDB::bind_method(D_METHOD("set_bitrate", "bitrate"), &OpusEncoder::set_bitrate);
    ClassDB::bind_method(D_METHOD("set_complexity", "complexity"), &OpusEncoder::set_complexity);
    ClassDB::bind_method(D_METHOD("set_signal_type", "signal_type"), &OpusEncoder::set_signal_type);
    ClassDB::bind_method(D_METHOD("get_sample_rate"), &OpusEncoder::get_sample_rate);
    ClassDB::bind_method(D_METHOD("get_channels"), &OpusEncoder::get_channels);
    ClassDB::bind_method(D_METHOD("get_frame_size"), &OpusEncoder::get_frame_size);
    ClassDB::bind_method(D_METHOD("is_initialized"), &OpusEncoder::is_initialized);
    ClassDB::bind_method(D_METHOD("reset"), &OpusEncoder::reset);
}

bool OpusEncoder::initialize(int bitrate) {
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
    
    int error;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
    
    if (error != OPUS_OK || !encoder) {
        UtilityFunctions::print("Failed to create Opus encoder: ", opus_strerror(error));
        return false;
    }
    
    // Set initial bitrate
    if (!set_bitrate(bitrate)) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
        return false;
    }
    
    UtilityFunctions::print("Opus encoder initialized successfully with bitrate: ", bitrate);
    return true;
}

PackedByteArray OpusEncoder::encode(const PackedByteArray& pcm_data) {
    PackedByteArray result;
    
    if (!encoder) {
        UtilityFunctions::print("Encoder not initialized");
        return result;
    }
    
    if (pcm_data.size() == 0) {
        UtilityFunctions::print("Empty PCM data");
        return result;
    }
    
    // PCM data should be 16-bit signed integers
    int samples_per_frame = FRAME_SIZE * CHANNELS;
    int bytes_per_frame = samples_per_frame * sizeof(int16_t);
    
    // Calculate number of complete frames and remaining bytes
    int complete_frames = pcm_data.size() / bytes_per_frame;
    int remaining_bytes = pcm_data.size() % bytes_per_frame;
    
    const int16_t* pcm_ptr = reinterpret_cast<const int16_t*>(pcm_data.ptr());
    
    // Prepare output buffer for all encoded frames
    PackedByteArray encoded_data;
    
    // Encode complete frames
    for (int frame = 0; frame < complete_frames; frame++) {
        unsigned char opus_packet[MAX_PACKET_SIZE];
        
        // Encode one frame
        int encoded_size = opus_encode(encoder, 
                                     pcm_ptr + (frame * samples_per_frame), 
                                     FRAME_SIZE, 
                                     opus_packet, 
                                     MAX_PACKET_SIZE);
        
        if (encoded_size < 0) {
            UtilityFunctions::print("Encoding failed: ", opus_strerror(encoded_size));
            return PackedByteArray(); // Return empty array on error
        }
        
        // Append encoded data
        int old_size = encoded_data.size();
        encoded_data.resize(old_size + encoded_size);
        memcpy(encoded_data.ptrw() + old_size, opus_packet, encoded_size);
    }
    
    // Handle remaining incomplete frame by padding with zeros
    if (remaining_bytes > 0) {
        // Create a buffer for the padded frame
        std::vector<int16_t> padded_frame(samples_per_frame, 0);
        
        // Copy remaining samples
        int remaining_samples = remaining_bytes / sizeof(int16_t);
        memcpy(padded_frame.data(), pcm_ptr + (complete_frames * samples_per_frame), remaining_bytes);
        
        // Encode the padded frame
        unsigned char opus_packet[MAX_PACKET_SIZE];
        int encoded_size = opus_encode(encoder, 
                                     padded_frame.data(), 
                                     FRAME_SIZE, 
                                     opus_packet, 
                                     MAX_PACKET_SIZE);
        
        if (encoded_size < 0) {
            UtilityFunctions::print("Warning: Failed to encode incomplete frame: ", opus_strerror(encoded_size));
            UtilityFunctions::print("Keeping PCM format for remaining ", remaining_bytes, " bytes (", remaining_samples, " samples)");
        } else {
            // Append encoded data
            int old_size = encoded_data.size();
            encoded_data.resize(old_size + encoded_size);
            memcpy(encoded_data.ptrw() + old_size, opus_packet, encoded_size);
            complete_frames++; // Count the padded frame as encoded
        }
    }
    
    // UtilityFunctions::print("Encoded ", complete_frames, " frames, total size: ", encoded_data.size(), " bytes");
    if (remaining_bytes > 0) {
        int remaining_samples = remaining_bytes / sizeof(int16_t);
        float remaining_ms = (float)remaining_samples / SAMPLE_RATE * 1000.0f;
        UtilityFunctions::print("Processed incomplete frame: ", remaining_bytes, " bytes (", remaining_samples, " samples, ", remaining_ms, " ms)");
    }
    
    return encoded_data;
}

bool OpusEncoder::set_bitrate(int bitrate) {
    if (!encoder) {
        UtilityFunctions::print("Encoder not initialized");
        return false;
    }
    
    int error = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
    if (error != OPUS_OK) {
        UtilityFunctions::print("Failed to set bitrate: ", opus_strerror(error));
        return false;
    }
    
    return true;
}

bool OpusEncoder::set_complexity(int complexity) {
    if (!encoder) {
        UtilityFunctions::print("Encoder not initialized");
        return false;
    }
    
    if (complexity < 0 || complexity > 10) {
        UtilityFunctions::print("Complexity must be between 0 and 10");
        return false;
    }
    
    int error = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));
    if (error != OPUS_OK) {
        UtilityFunctions::print("Failed to set complexity: ", opus_strerror(error));
        return false;
    }
    
    return true;
}

bool OpusEncoder::set_signal_type(int signal_type) {
    if (!encoder) {
        UtilityFunctions::print("Encoder not initialized");
        return false;
    }
    
    if (signal_type != OPUS_SIGNAL_VOICE && signal_type != OPUS_SIGNAL_MUSIC) {
        UtilityFunctions::print("Invalid signal type. Use OPUS_SIGNAL_VOICE or OPUS_SIGNAL_MUSIC");
        return false;
    }
    
    int error = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(signal_type));
    if (error != OPUS_OK) {
        UtilityFunctions::print("Failed to set signal type: ", opus_strerror(error));
        return false;
    }
    
    return true;
}

void OpusEncoder::reset() {
    if (!encoder) {
        UtilityFunctions::print("Encoder not initialized");
        return;
    }
    
    int error = opus_encoder_ctl(encoder, OPUS_RESET_STATE);
    if (error != OPUS_OK) {
        UtilityFunctions::print("Failed to reset encoder: ", opus_strerror(error));
    }
} 