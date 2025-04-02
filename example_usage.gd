extends Node

# DiffDetector GDExtension示例使用脚本

# 差异检测器实例
var diff_detector: DiffDetector

# 图像资源
var source_image: Image
var modified_image: Image

# 差异点数据
var diff_data = []

func _ready():
	# 创建差异检测器实例
	diff_detector = DiffDetector.new()
	
	# 初始化检测器 (加载模型)
	if not diff_detector.initialize():
		print_debug("无法初始化DiffDetector")
		return
	
	# 加载测试图像
	var test_image_path = "res://test_image.png"
	source_image = Image.load_from_file(test_image_path)
	if not source_image:
		print_debug("无法加载图像:", test_image_path)
		return
	
	# 设置差异生成参数
	diff_detector.diff_count = 7  # 差异点数量，范围5-10
	diff_detector.difficulty = 3  # 难度，范围1-10
	
	# 生成差异图像
	modified_image = diff_detector.generate_diff_image(source_image, 7, 3)
	
	# 获取差异点数据
	diff_data = diff_detector.get_diff_data()
	
	# 打印差异点信息
	print("生成了 ", diff_data.size(), " 个差异点:")
	for i in range(diff_data.size()):
		var diff = diff_data[i]
		print("差异点 #", i+1, ":")
		print("  位置: ", diff.position)
		print("  大小: ", diff.size)
		print("  算法ID: ", diff.algorithm_id)
	
	# 保存结果图像（可选）
	modified_image.save_png("res://modified_image.png")
	
	# 在游戏中展示图像（示例）
	display_images()

# 在UI中显示原始和修改后的图像
func display_images():
	# 创建两个TextureRect来显示原始和修改后的图像
	var original_texture = ImageTexture.create_from_image(source_image)
	var modified_texture = ImageTexture.create_from_image(modified_image)
	
	# 原始图像显示
	var original_rect = TextureRect.new()
	original_rect.texture = original_texture
	original_rect.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	original_rect.custom_minimum_size = Vector2(400, 300)
	
	# 修改后图像显示
	var modified_rect = TextureRect.new()
	modified_rect.texture = modified_texture
	modified_rect.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	modified_rect.custom_minimum_size = Vector2(400, 300)
	
	# 水平排列的容器
	var hbox = HBoxContainer.new()
	hbox.add_child(original_rect)
	hbox.add_child(modified_rect)
	
	# 添加到场景
	add_child(hbox) 