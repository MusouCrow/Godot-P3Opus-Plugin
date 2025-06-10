#include "opus_session_decoder.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <opus.h>
#include <cstring>

using namespace godot;

void OpusSessionDecoder::_bind_methods() {
    // Session management
    ClassDB::bind_method(D_METHOD("start_session"), &OpusSessionDecoder::start_session);
    ClassDB::bind_method(D_METHOD("end_session"), &OpusSessionDecoder::end_session);
    ClassDB::bind_method(D_METHOD("reset_session"), &OpusSessionDecoder::reset_session);
    ClassDB::bind_method(D_METHOD("is_session_active"), &OpusSessionDecoder::is_session_active);
    
    // Decoding methods
    ClassDB::bind_method(D_METHOD("decode_packet", "opus_data"), &OpusSessionDecoder::decode_packet);
    ClassDB::bind_method(D_METHOD("decode_packets", "opus_packets"), &OpusSessionDecoder::decode_packets);
    
    // Statistics and info
    ClassDB::bind_method(D_METHOD("get_total_decoded_samples"), &OpusSessionDecoder::get_total_decoded_samples);
    ClassDB::bind_method(D_METHOD("get_total_decoded_duration"), &OpusSessionDecoder::get_total_decoded_duration);
    ClassDB::bind_method(D_METHOD("get_packet_count"), &OpusSessionDecoder::get_packet_count);
    ClassDB::bind_method(D_METHOD("reset_statistics"), &OpusSessionDecoder::reset_statistics);
    
    // Audio parameters
    ClassDB::bind_method(D_METHOD("get_sample_rate"), &OpusSessionDecoder::get_sample_rate);
    ClassDB::bind_method(D_METHOD("get_channels"), &OpusSessionDecoder::get_channels);
    ClassDB::bind_method(D_METHOD("get_max_frame_size"), &OpusSessionDecoder::get_max_frame_size);
}

OpusSessionDecoder::OpusSessionDecoder() {
    decoder = nullptr;
    session_active = false;
    total_decoded_samples = 0;
    packet_count = 0;
}

OpusSessionDecoder::~OpusSessionDecoder() {
    end_session();
}

// ========== Session Management ==========

bool OpusSessionDecoder::start_session() {
    // 如果已经有活跃会话，先结束
    if (session_active) {
        end_session();
    }
    
    int error;
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    
    if (error != OPUS_OK) {
        UtilityFunctions::print("OpusSessionDecoder: Failed to create decoder: ", opus_strerror(error));
        decoder = nullptr;
        session_active = false;
        return false;
    }
    
    session_active = true;
    total_decoded_samples = 0;
    packet_count = 0;
    
    UtilityFunctions::print("OpusSessionDecoder: Session started (", SAMPLE_RATE, "Hz, ", CHANNELS, " channel)");
    return true;
}

void OpusSessionDecoder::end_session() {
    if (decoder != nullptr) {
        opus_decoder_destroy(decoder);
        decoder = nullptr;
    }
    
    if (session_active) {
        UtilityFunctions::print("OpusSessionDecoder: Session ended (processed ", packet_count, " packets, ", 
                               total_decoded_samples, " samples, ", get_total_decoded_duration(), " seconds)");
    }
    
    session_active = false;
}

void OpusSessionDecoder::reset_session() {
    if (!session_active || decoder == nullptr) {
        UtilityFunctions::print("OpusSessionDecoder: No active session to reset");
        return;
    }
    
    // 重置解码器状态但不销毁
    int error = opus_decoder_ctl(decoder, OPUS_RESET_STATE);
    if (error != OPUS_OK) {
        UtilityFunctions::print("OpusSessionDecoder: Failed to reset decoder state: ", opus_strerror(error));
    } else {
        UtilityFunctions::print("OpusSessionDecoder: Decoder state reset");
    }
    
    // 可选择是否重置统计信息
    // reset_statistics();
}

bool OpusSessionDecoder::is_session_active() const {
    return session_active && decoder != nullptr;
}

// ========== Decoding Methods ==========

PackedByteArray OpusSessionDecoder::decode_packet(const PackedByteArray& opus_data) {
    PackedByteArray result;
    
    if (!session_active || decoder == nullptr) {
        UtilityFunctions::print("OpusSessionDecoder: No active session. Call start_session() first.");
        return result;
    }
    
    if (opus_data.size() == 0) {
        UtilityFunctions::print("OpusSessionDecoder: Empty opus data");
        return result;
    }
    
    // 分配解码缓冲区
    opus_int16* pcm_buffer = new opus_int16[MAX_FRAME_SIZE * CHANNELS];
    
    // 解码 Opus 包
    int decoded_samples = opus_decode(decoder, opus_data.ptr(), opus_data.size(), pcm_buffer, MAX_FRAME_SIZE, 0);
    
    if (decoded_samples > 0) {
        // 转换为 PackedByteArray
        int pcm_bytes = decoded_samples * CHANNELS * sizeof(opus_int16);
        result.resize(pcm_bytes);
        memcpy(result.ptrw(), pcm_buffer, pcm_bytes);
        
        // 更新统计信息
        total_decoded_samples += decoded_samples;
        packet_count++;
        
    } else if (decoded_samples < 0) {
        UtilityFunctions::print("OpusSessionDecoder: Decode failed: ", opus_strerror(decoded_samples));
    }
    
    delete[] pcm_buffer;
    return result;
}

PackedByteArray OpusSessionDecoder::decode_packets(const Array& opus_packets) {
    PackedByteArray result;
    
    if (!session_active || decoder == nullptr) {
        UtilityFunctions::print("OpusSessionDecoder: No active session. Call start_session() first.");
        return result;
    }
    
    if (opus_packets.size() == 0) {
        UtilityFunctions::print("OpusSessionDecoder: Empty packets array");
        return result;
    }
    
    UtilityFunctions::print("OpusSessionDecoder: Decoding ", opus_packets.size(), " packets...");
    
    // 分配解码缓冲区
    opus_int16* pcm_buffer = new opus_int16[MAX_FRAME_SIZE * CHANNELS];
    PackedByteArray temp_pcm_data;
    
    int success_count = 0;
    int64_t batch_samples = 0;
    
    // 批量解码
    for (int i = 0; i < opus_packets.size(); i++) {
        Variant packet_variant = opus_packets[i];
        
        if (packet_variant.get_type() != Variant::PACKED_BYTE_ARRAY) {
            UtilityFunctions::print("OpusSessionDecoder: Packet ", i, " is not PackedByteArray, skipping");
            continue;
        }
        
        PackedByteArray opus_packet = packet_variant;
        if (opus_packet.size() == 0) {
            continue;
        }
        
        // 解码包
        int decoded_samples = opus_decode(decoder, opus_packet.ptr(), opus_packet.size(), pcm_buffer, MAX_FRAME_SIZE, 0);
        
        if (decoded_samples > 0) {
            // 添加到结果
            int pcm_bytes = decoded_samples * CHANNELS * sizeof(opus_int16);
            int old_size = temp_pcm_data.size();
            temp_pcm_data.resize(old_size + pcm_bytes);
            memcpy(temp_pcm_data.ptrw() + old_size, pcm_buffer, pcm_bytes);
            
            success_count++;
            batch_samples += decoded_samples;
            
            if ((i + 1) % 50 == 0) {
                UtilityFunctions::print("OpusSessionDecoder: Processed ", i + 1, "/", opus_packets.size(), " packets");
            }
        } else if (decoded_samples < 0) {
            UtilityFunctions::print("OpusSessionDecoder: Packet ", i, " decode failed: ", opus_strerror(decoded_samples));
        }
    }
    
    // 更新统计信息
    total_decoded_samples += batch_samples;
    packet_count += success_count;
    
    UtilityFunctions::print("OpusSessionDecoder: Batch complete - ", success_count, "/", opus_packets.size(), 
                           " packets successful, ", batch_samples, " samples decoded");
    
    delete[] pcm_buffer;
    return temp_pcm_data;
}

// ========== Statistics and Info ==========

int64_t OpusSessionDecoder::get_total_decoded_samples() const {
    return total_decoded_samples;
}

double OpusSessionDecoder::get_total_decoded_duration() const {
    return (double)total_decoded_samples / SAMPLE_RATE;
}

int OpusSessionDecoder::get_packet_count() const {
    return packet_count;
}

void OpusSessionDecoder::reset_statistics() {
    total_decoded_samples = 0;
    packet_count = 0;
    UtilityFunctions::print("OpusSessionDecoder: Statistics reset");
} 