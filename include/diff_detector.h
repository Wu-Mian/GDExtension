#ifndef DIFF_DETECTOR_H
#define DIFF_DETECTOR_H

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <opencv2/core.hpp>
#include <memory>

namespace godot {

// 前向声明
class DiffGenerator;
class YoloDetector;

// 差异点信息结构
struct DiffInfo {
    Vector2 position;   // 坐标
    float size;         // 大小
    int algorithm_id;   // 使用的差异算法ID
};

// 主要GDExtension类
class DiffDetector : public RefCounted {
    GDCLASS(DiffDetector, RefCounted);

private:
    std::unique_ptr<DiffGenerator> diff_generator;
    std::unique_ptr<YoloDetector> yolo_detector;
    
    // 差异生成参数
    int diff_count;     // 差异点数量
    int difficulty;     // 难度参数

    std::vector<DiffInfo> generated_diffs; // 生成的差异信息

protected:
    static void _bind_methods();

public:
    DiffDetector();
    ~DiffDetector();

    // Godot接口方法
    bool initialize();
    Ref<Image> generate_diff_image(const Ref<Image>& source_image, int diff_count, int difficulty);
    Array get_diff_data() const;
    
    // 设置/获取参数
    void set_diff_count(int count);
    int get_diff_count() const;
    
    void set_difficulty(int diff);
    int get_difficulty() const;
};

}  // namespace godot

#endif // DIFF_DETECTOR_H 