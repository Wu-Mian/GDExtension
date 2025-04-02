#ifndef DIFF_GENERATOR_H
#define DIFF_GENERATOR_H

#include <vector>
#include <functional>
#include <random>
#include <opencv2/core.hpp>
#include "diff_detector.h"

namespace godot {

// 差异生成器类，负责在图像中创建差异效果
class DiffGenerator {
public:
    // 差异算法类型枚举
    enum DiffAlgorithmType {
        DIFF_COLOR_SHIFT = 0,       // 颜色变化
        DIFF_OBJECT_REMOVAL = 1,    // 物体删除
        DIFF_TEXTURE_CHANGE = 2,    // 纹理变化
        DIFF_SHAPE_DEFORM = 3,      // 形状变形
        DIFF_SUBTLE_PATTERN = 4,    // 细微图案变化
        DIFF_SCALE_CHANGE = 5,      // 大小缩放
        DIFF_ROTATION = 6,          // 旋转
        DIFF_FLIP = 7,              // 翻转
        DIFF_BLUR = 8,              // 模糊
        DIFF_ADDITION = 9           // 添加小物体
    };

private:
    // 随机数生成器
    std::mt19937 rng;
    
    // 差异算法函数映射
    std::vector<std::function<void(cv::Mat&, const cv::Rect&, int, DiffInfo&)>> diff_algorithms;
    
    // 选择可应用差异的区域
    std::vector<cv::Rect> select_diff_regions(const cv::Mat& image, 
                                             const std::vector<DetectedObject>& objects,
                                             int count);
    
    // 应用特定差异算法到区域
    void apply_diff_algorithm(cv::Mat& image, const cv::Rect& region, int difficulty, 
                             DiffInfo& diff_info);

    // 各种差异算法的实现
    void apply_color_shift(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_object_removal(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_texture_change(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_shape_deform(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_subtle_pattern(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_scale_change(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_rotation(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_flip(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_blur(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);
    void apply_addition(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info);

public:
    DiffGenerator();
    ~DiffGenerator();

    // 生成差异
    bool generate_diffs(cv::Mat& image, const std::vector<DetectedObject>& objects, 
                       int count, int difficulty, std::vector<DiffInfo>& diff_info);
                       
    // 选择适合难度的差异算法
    int select_algorithm_for_difficulty(int difficulty);
};

} // namespace godot

#endif // DIFF_GENERATOR_H 