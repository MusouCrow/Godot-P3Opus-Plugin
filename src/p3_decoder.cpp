#include "p3_decoder.h"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <opus.h>
#include <cstring>

using namespace godot;

void P3Decoder::_bind_methods() {
    ClassDB::bind_method(D_METHOD("decode_p3_file", "file_path"), &P3Decoder::decode_p3_file);
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

bool P3Decoder::is_p3_file(const String& filename) {
    return filename.get_extension().to_lower() == "p3";
}

PackedByteArray P3Decoder::decode_p3_file(const String& file_path) {
    PackedByteArray result;
    
    // 检查文件扩展名
    if (!is_p3_file(file_path)) {
        UtilityFunctions::print("错误: 只支持 .p3 格式文件");
        return result;
    }
    
    // 打开文件
    Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::READ);
    if (file.is_null()) {
        UtilityFunctions::print("错误: 无法打开文件 '", file_path, "'");
        return result;
    }
    
    // 初始化Opus解码器
    int error;
    OpusDecoder* decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        UtilityFunctions::print("错误: 无法创建Opus解码器: ", opus_strerror(error));
        file->close();
        return result;
    }
    
    UtilityFunctions::print("开始解码 p3 文件: ", file_path);
    UtilityFunctions::print("采样率: ", SAMPLE_RATE, " Hz, 声道数: ", CHANNELS);
    
    // 分配缓冲区
    opus_int16* pcm_buffer = new opus_int16[FRAME_SIZE * CHANNELS];
    unsigned char* opus_buffer = new unsigned char[4096];
    
    // 用于存储所有PCM数据的临时数组
    PackedByteArray temp_pcm_data;
    
    int packet_count = 0;
    int64_t total_pcm_samples = 0;
    
    // 逐包解码
    while (!file->eof_reached()) {
        // 检查是否还有足够的数据读取头部
        if (file->get_position() + sizeof(P3Header) > file->get_length()) {
            break;
        }
        
        // 读取p3头部 (4字节)
        P3Header header;
        PackedByteArray header_data = file->get_buffer(sizeof(P3Header));
        if (header_data.size() != sizeof(P3Header)) {
            break;
        }
        
        // 复制头部数据
        memcpy(&header, header_data.ptr(), sizeof(P3Header));
        
        // 转换大端序长度字段
        uint16_t data_len = be16_to_host(header.data_len);
        
        if (data_len == 0 || data_len > 4096) {
            UtilityFunctions::print("错误: 无效的数据长度 ", data_len);
            break;
        }
        
        // 读取Opus数据
        PackedByteArray opus_data = file->get_buffer(data_len);
        if (opus_data.size() != data_len) {
            UtilityFunctions::print("错误: Opus数据读取不完整 (期望 ", data_len, " 字节，实际 ", opus_data.size(), " 字节)");
            break;
        }
        
        // 复制到opus_buffer
        memcpy(opus_buffer, opus_data.ptr(), data_len);
        
        // 解码Opus数据
        int decoded_samples = opus_decode(decoder, opus_buffer, data_len, pcm_buffer, FRAME_SIZE, 0);
        if (decoded_samples < 0) {
            UtilityFunctions::print("错误: Opus解码失败: ", opus_strerror(decoded_samples));
            break;
        }
        
        // 将PCM数据添加到结果中
        int pcm_bytes = decoded_samples * CHANNELS * sizeof(opus_int16);
        int old_size = temp_pcm_data.size();
        temp_pcm_data.resize(old_size + pcm_bytes);
        memcpy(temp_pcm_data.ptrw() + old_size, pcm_buffer, pcm_bytes);
        
        packet_count++;
        total_pcm_samples += decoded_samples;
        
        if (packet_count % 100 == 0) {
            UtilityFunctions::print("已处理 ", packet_count, " 个数据包...");
        }
    }
    
    UtilityFunctions::print("解码完成!");
    UtilityFunctions::print("总共处理了 ", packet_count, " 个数据包");
    UtilityFunctions::print("总PCM采样数: ", total_pcm_samples);
    UtilityFunctions::print("音频时长: ", (double)total_pcm_samples / SAMPLE_RATE, " 秒");
    
    // 清理资源
    delete[] pcm_buffer;
    delete[] opus_buffer;
    opus_decoder_destroy(decoder);
    file->close();
    
    return temp_pcm_data;
} 