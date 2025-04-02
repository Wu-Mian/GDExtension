#ifndef DIFF_GENERATOR_H
#define DIFF_GENERATOR_H

#include <vector>
#include <string>
#include <random>
#include <map>
#include <functional>
#include <opencv2/core.hpp>
#include "yolo_detector.h"

// 差异类型枚举
enum DiffType {
    DIFF_COLOR_SHIFT = 0,
    DIFF_OBJECT_REMOVAL = 1,
    DIFF_TEXTURE_CHANGE = 2,
    DIFF_SHAPE_DEFORM = 3,
    DIFF_SUBTLE_PATTERN = 4,
    DIFF_SCALE_CHANGE = 5,
    DIFF_ROTATION = 6,
    DIFF_FLIP = 7,
    DIFF_BLUR = 8,
    DIFF_ADDITION = 9
};

// 差异信息结构体
struct DiffInfo {
    cv::Point position;         // 差异位置
    cv::Size size;              // 差异大小
    DiffType algorithm_id;      // 使用的算法ID
    cv::Rect region;            // 差异区域
};

/**
 * 差异生成器类
 * 负责生成图像差异
 */
class DiffGenerator {
public:
    DiffGenerator();
    ~DiffGenerator();

    /**
     * 生成图像差异
     * @param image 原始图像
     * @param diff_count 差异数量
     * @param difficulty 难度级别 (1-10)
     * @param detections 检测到的物体
     * @param diff_info 输出的差异信息
     * @return 成功返回true，失败返回false
     */
    bool generate_diffs(cv::Mat& image, int diff_count, int difficulty, 
                      const std::vector<DetectedObject>& detections,
                      std::vector<DiffInfo>& diff_info);

private:
    std::mt19937 rng;  // 随机数生成器
    std::map<DiffType, std::function<void(cv::Mat&, const cv::Rect&, int, DiffInfo&)>> diff_algorithms;

    /**
     * 选择差异区域
     * @param image 图像
     * @param diff_count 差异数量
     * @param detections 检测到的物体
     * @return 选择的区域列表
     */
    std::vector<cv::Rect> select_diff_regions(const cv::Mat& image, int diff_count,
                                           const std::vector<DetectedObject>& detections);

    /**
     * 根据难度选择算法
     * @param difficulty 难度级别
     * @return 选择的算法类型
     */
    DiffType select_algorithm_for_difficulty(int difficulty);

    /**
     * 应用差异算法
     * @param image 图像
     * @param region 区域
     * @param difficulty 难度级别
     * @param algorithm_id 算法ID
     * @param diff_info 输出的差异信息
     */
    void apply_diff_algorithm(cv::Mat& image, const cv::Rect& region, int difficulty, 
                           DiffType algorithm_id, DiffInfo& diff_info);

    // 各种差异算法
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
};

#endif // DIFF_GENERATOR_H 