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

- CMake 3.12+
- 支持C++17的编译器
- Godot 4.4
- Android NDK (用于Android构建)
- Xcode (用于iOS构建)

### 外部依赖

- OpenCV-mobile
- LiteRT
- godot-cpp (Godot 4.4)

## 编译步骤

### Android

```bash
mkdir -p build/android
cd build/android
cmake -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DCMAKE_BUILD_TYPE=Release \
      ../..
make -j8
```

### iOS

```bash
mkdir -p build/ios
cd build/ios
cmake -GXcode \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
      -DCMAKE_BUILD_TYPE=Release \
      ../..
xcodebuild -project DiffDetectorGDExtension.xcodeproj -configuration Release -sdk iphoneos
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
- LiteRT: 用于移动设备的轻量级深度学习推理引擎
- YOLOv11: 最新的目标检测与分割模型 