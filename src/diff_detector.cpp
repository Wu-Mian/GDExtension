#include "diff_detector.h"
#include "diff_generator.h"
#include "yolo_detector.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <opencv2/imgproc.hpp>

#include <filesystem>

namespace godot {

DiffDetector::DiffDetector() : diff_count(5), difficulty(1) {
    diff_generator = std::make_unique<DiffGenerator>();
    yolo_detector = std::make_unique<YoloDetector>();
}

DiffDetector::~DiffDetector() {
    // 智能指针会自动清理资源
}

bool DiffDetector::initialize() {
    // 获取模型文件路径（从Godot项目目录）
    std::string model_path = "res://bin/";
    
    #ifdef __ANDROID__
    model_path += "android/";
    #elif defined(__APPLE__)
    model_path += "ios/";
    #endif
    
    model_path += "assets/yolo11s-seg_float16.tflite";
    
    // 初始化YOLO检测器
    if (!yolo_detector->initialize(model_path)) {
        UtilityFunctions::print("Failed to initialize YOLO detector with model: ", model_path.c_str());
        return false;
    }
    
    UtilityFunctions::print("DiffDetector initialized successfully");
    return true;
}

Ref<Image> DiffDetector::generate_diff_image(const Ref<Image>& source_image, int count, int diff) {
    // 参数验证
    if (source_image.is_null()) {
        UtilityFunctions::print_error("Source image is null");
        return source_image;
    }
    
    // 限制差异点数量
    if (count < 5) count = 5;
    if (count > 10) count = 10;
    
    // 设置内部参数
    set_diff_count(count);
    set_difficulty(diff);
    
    // 将Godot图像转换为OpenCV格式
    PackedByteArray image_data = source_image->get_data();
    int width = source_image->get_width();
    int height = source_image->get_height();
    int components = source_image->get_format() >= Image::FORMAT_RGB8 ? 3 : 4;
    
    cv::Mat cv_image(height, width, components == 3 ? CV_8UC3 : CV_8UC4, image_data.ptrw());
    
    // 如果是RGBA，需要转换为RGB
    if (components == 4) {
        cv::cvtColor(cv_image, cv_image, cv::COLOR_RGBA2RGB);
    }
    
    // 运行YOLO检测
    if (!yolo_detector->detect(cv_image)) {
        UtilityFunctions::print_error("YOLO detection failed");
        return source_image;
    }
    
    // 获取检测结果
    const std::vector<DetectedObject>& detections = yolo_detector->get_detections();
    
    // 生成差异（修改cv_image）
    generated_diffs.clear();
    bool diff_result = diff_generator->generate_diffs(cv_image, detections, diff_count, difficulty, generated_diffs);
    
    if (!diff_result) {
        UtilityFunctions::print_error("Failed to generate differences");
        return source_image;
    }
    
    // 如果需要转回RGBA格式
    if (components == 4) {
        cv::cvtColor(cv_image, cv_image, cv::COLOR_RGB2RGBA);
    }
    
    // 创建修改后的Godot图像
    Ref<Image> modified_image = source_image->duplicate();
    PackedByteArray modified_data;
    modified_data.resize(image_data.size());
    
    // 复制OpenCV图像数据到Godot图像
    memcpy(modified_data.ptrw(), cv_image.data, cv_image.total() * cv_image.elemSize());
    modified_image->set_data(width, height, false, source_image->get_format(), modified_data);
    
    return modified_image;
}

Array DiffDetector::get_diff_data() const {
    Array result;
    
    for (const auto& diff : generated_diffs) {
        Dictionary diff_dict;
        diff_dict["position"] = diff.position;
        diff_dict["size"] = diff.size;
        diff_dict["algorithm_id"] = diff.algorithm_id;
        result.push_back(diff_dict);
    }
    
    return result;
}

void DiffDetector::set_diff_count(int count) {
    diff_count = count;
    if (diff_count < 5) diff_count = 5;
    if (diff_count > 10) diff_count = 10;
}

int DiffDetector::get_diff_count() const {
    return diff_count;
}

void DiffDetector::set_difficulty(int diff) {
    difficulty = diff;
    if (difficulty < 1) difficulty = 1;
    if (difficulty > 10) difficulty = 10;
}

int DiffDetector::get_difficulty() const {
    return difficulty;
}

void DiffDetector::_bind_methods() {
    // 注册方法
    ClassDB::bind_method(D_METHOD("initialize"), &DiffDetector::initialize);
    ClassDB::bind_method(D_METHOD("generate_diff_image", "source_image", "diff_count", "difficulty"), &DiffDetector::generate_diff_image);
    ClassDB::bind_method(D_METHOD("get_diff_data"), &DiffDetector::get_diff_data);
    
    // 注册属性访问方法
    ClassDB::bind_method(D_METHOD("set_diff_count", "count"), &DiffDetector::set_diff_count);
    ClassDB::bind_method(D_METHOD("get_diff_count"), &DiffDetector::get_diff_count);
    ClassDB::bind_method(D_METHOD("set_difficulty", "difficulty"), &DiffDetector::set_difficulty);
    ClassDB::bind_method(D_METHOD("get_difficulty"), &DiffDetector::get_difficulty);
    
    // 暴露属性
    ADD_PROPERTY(PropertyInfo(Variant::INT, "diff_count", PROPERTY_HINT_RANGE, "5,10,1"), "set_diff_count", "get_diff_count");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "difficulty", PROPERTY_HINT_RANGE, "1,10,1"), "set_difficulty", "get_difficulty");
}

} // namespace godot 