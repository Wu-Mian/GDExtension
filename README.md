# DiffDetector GDExtension

基于OpenCV-mobile与LiteRT的Godot 4.4 GDExtension，用于找茬游戏的图像差异生成。通过结合YOLOv11模型的目标检测与分割能力和OpenCV的图像处理功能，实现智能的图像差异生成。

## 特性

- 结合YOLOv11目标检测与分割能力
- 多种差异生成算法
- 可调节难度参数
- 支持Android和iOS移动平台
- 优化用于找茬游戏开发
- 返回详细的差异点元数据

## 安装

1. 将整个DiffDetector文件夹复制到您的Godot项目中
2. 确保`diff_detector.gdextension`文件位于项目的根目录
3. 将预训练的模型文件`yolo11s-seg_float16.tflite`放置在`bin/android/assets/`和`bin/ios/assets/`目录下

## 构建要求

- Python 3.x
- SCons 4.0+
- Godot 4.4
- Android NDK (用于Android构建)
- Xcode (用于iOS构建)
- Docker (用于构建LiteRT)
- Bazel (用于构建LiteRT)

### 外部依赖

- OpenCV-mobile (预编译)
- LiteRT (从源代码构建)
- godot-cpp (源代码，通过git submodule管理)

## 依赖设置

项目使用自动化脚本管理依赖。运行以下命令设置所有依赖：

```bash
# 安装Python依赖
pip install scons

# 安装Docker和Bazel
# Ubuntu/Debian:
sudo apt-get update
sudo apt-get install -y docker.io bazel
sudo usermod -aG docker $USER

# macOS:
brew install docker docker-compose bazel

# Windows:
# 安装Docker Desktop和Chocolatey，然后：
choco install bazel

# 设置项目依赖
python scripts/setup_dependencies.py
```

此脚本会：
1. 克隆并设置godot-cpp子模块
2. 下载并解压OpenCV-mobile预编译库
3. 克隆并构建LiteRT（使用Docker和Bazel）

### LiteRT构建说明

LiteRT使用Docker容器进行构建，确保构建环境的一致性。构建过程包括：
1. 克隆LiteRT仓库
2. 初始化子模块
3. 构建Docker镜像
4. 在容器中构建LiteRT
5. 复制构建产物到正确位置

## 编译步骤

### Android构建

```bash
# 设置Android NDK路径
export ANDROID_NDK_ROOT=/path/to/android-ndk

# 构建
scons platform=android
```

### iOS构建

```bash
scons platform=ios
```

### Windows构建

```bash
scons platform=windows
```

## 使用方法

```gdscript
# 创建差异检测器实例
var diff_detector = DiffDetector.new()

# 初始化检测器
diff_detector.initialize()

# 加载图像
var source_image = Image.load_from_file("res://my_image.png")

# 设置参数
diff_detector.diff_count = 7  # 5-10之间
diff_detector.difficulty = 5  # 1-10之间 (越高越难)

# 生成差异图像
var modified_image = diff_detector.generate_diff_image(source_image, 7, 5)

# 获取差异点数据
var diff_data = diff_detector.get_diff_data()

# 使用差异数据 (例如，高亮显示差异点或实现游戏逻辑)
for diff in diff_data:
    var position = diff.position  # 差异点位置向量
    var size = diff.size          # 差异的大小
    var algorithm_id = diff.algorithm_id  # 使用的差异算法ID
```

## 差异算法类型

DiffGenerator提供以下差异算法类型：

- `DIFF_COLOR_SHIFT` (0): 颜色变化
- `DIFF_OBJECT_REMOVAL` (1): 物体删除
- `DIFF_TEXTURE_CHANGE` (2): 纹理变化
- `DIFF_SHAPE_DEFORM` (3): 形状变形
- `DIFF_SUBTLE_PATTERN` (4): 细微图案变化
- `DIFF_SCALE_CHANGE` (5): 大小缩放
- `DIFF_ROTATION` (6): 旋转
- `DIFF_FLIP` (7): 翻转
- `DIFF_BLUR` (8): 模糊
- `DIFF_ADDITION` (9): 添加小物体

## 难度参数

- 1-3: 简单难度，产生明显差异 (颜色变化、物体删除等)
- 4-7: 中等难度，产生中等明显的差异 (纹理变化、形状变形等)
- 8-10: 高难度，产生微妙、难以察觉的差异 (细微图案、微小形状变化等)

## 许可证

本项目使用MIT许可证。

## 致谢

- OpenCV-mobile: https://github.com/nihui/opencv-mobile
- LiteRT: https://github.com/google-ai-edge/LiteRT
- YOLOv11: 最新的目标检测与分割模型 