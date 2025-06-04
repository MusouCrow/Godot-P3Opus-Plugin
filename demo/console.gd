extends Node

class_name Console

func _ready():
	var file_path = "res://voice.p3"
	
	# 创建P3解码器并解码文件
	var decoder = P3Decoder.new()
	var pcm_data = decoder.decode_p3_file(file_path)
	
	if pcm_data.size() == 0:
		print("解码失败")
		return

	# 创建音频流
	var stream = AudioStreamWAV.new()
	stream.format = AudioStreamWAV.FORMAT_16_BITS
	stream.mix_rate = decoder.get_sample_rate()  # 16000
	stream.stereo = false  # 单声道
	stream.data = pcm_data

	# 播放音频
	var player = AudioStreamPlayer.new()
	player.stream = stream
	add_child(player)
	player.play() 
