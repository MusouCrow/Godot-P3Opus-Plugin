#include "p3_decoder.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <opus.h>
#include <cstring>

using namespace godot;

void P3Decoder::_bind_methods() {
    ClassDB::bind_method(D_METHOD("decode_p3", "p3_data"), &P3Decoder::decode_p3);
    ClassDB::bind_method(D_METHOD("get_sample_rate"), &P3Decoder::get_sample_rate);
    ClassDB::bind_method(D_METHOD("get_channels"), &P3Decoder::get_channels);
}

P3Decoder::P3Decoder() {
}

P3Decoder::~P3Decoder() {
}

uint16_t P3Decoder::be16_to_host(uint16_t value) {
    return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
}

PackedByteArray P3Decoder::decode_p3(const PackedByteArray& p3_data) {
    PackedByteArray result;
    
    if (p3_data.size() == 0) {
        UtilityFunctions::print("Error: Input binary data is empty");
        return result;
    }
    
    // Initialize Opus decoder
    int error;
    OpusDecoder* decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        UtilityFunctions::print("Error: Failed to create Opus decoder: ", opus_strerror(error));
        return result;
    }
    
    UtilityFunctions::print("Starting to decode p3 data stream");
    UtilityFunctions::print("Data size: ", p3_data.size(), " bytes");
    UtilityFunctions::print("Sample rate: ", SAMPLE_RATE, " Hz, Channels: ", CHANNELS);
    
    // Allocate buffers
    opus_int16* pcm_buffer = new opus_int16[FRAME_SIZE * CHANNELS];
    unsigned char* opus_buffer = new unsigned char[4096];
    
    // Temporary array to store all PCM data
    PackedByteArray temp_pcm_data;
    
    int packet_count = 0;
    int64_t total_pcm_samples = 0;
    int64_t data_pos = 0;
    const uint8_t* data_ptr = p3_data.ptr();
    int64_t data_size = p3_data.size();
    
    // Decode packet by packet
    while (data_pos < data_size) {
        // Check if there's enough data to read the header
        if (data_pos + sizeof(P3Header) > data_size) {
            break;
        }
        
        // Read p3 header (4 bytes)
        P3Header header;
        memcpy(&header, data_ptr + data_pos, sizeof(P3Header));
        data_pos += sizeof(P3Header);
        
        // Convert big endian length field
        uint16_t data_len = be16_to_host(header.data_len);
        
        if (data_len == 0 || data_len > 4096) {
            UtilityFunctions::print("Error: Invalid data length ", data_len);
            break;
        }
        
        // Check if there's enough data
        if (data_pos + data_len > data_size) {
            UtilityFunctions::print("Error: Incomplete Opus data (expected ", data_len, " bytes, remaining ", data_size - data_pos, " bytes)");
            break;
        }
        
        // Copy Opus data to buffer
        memcpy(opus_buffer, data_ptr + data_pos, data_len);
        data_pos += data_len;
        
        // Decode Opus data
        int decoded_samples = opus_decode(decoder, opus_buffer, data_len, pcm_buffer, FRAME_SIZE, 0);
        if (decoded_samples < 0) {
            UtilityFunctions::print("Error: Opus decoding failed: ", opus_strerror(decoded_samples));
            break;
        }
        
        // Add PCM data to result
        int pcm_bytes = decoded_samples * CHANNELS * sizeof(opus_int16);
        int old_size = temp_pcm_data.size();
        temp_pcm_data.resize(old_size + pcm_bytes);
        memcpy(temp_pcm_data.ptrw() + old_size, pcm_buffer, pcm_bytes);
        
        packet_count++;
        total_pcm_samples += decoded_samples;
        
        if (packet_count % 100 == 0) {
            UtilityFunctions::print("Processed ", packet_count, " packets...");
        }
    }
    
    UtilityFunctions::print("Decoding completed!");
    UtilityFunctions::print("Total processed packets: ", packet_count);
    UtilityFunctions::print("Total PCM samples: ", total_pcm_samples);
    UtilityFunctions::print("Audio duration: ", (double)total_pcm_samples / SAMPLE_RATE, " seconds");
    
    // Clean up resources
    delete[] pcm_buffer;
    delete[] opus_buffer;
    opus_decoder_destroy(decoder);
    
    return temp_pcm_data;
} 