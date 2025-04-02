#include "diff_generator.h"
#include "yolo_detector.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <algorithm>
#include <random>
#include <chrono>

namespace godot {

DiffGenerator::DiffGenerator() {
    // 初始化随机数生成器
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng = std::mt19937(seed);
    
    // 初始化差异算法函数映射
    diff_algorithms.resize(10);
    diff_algorithms[DIFF_COLOR_SHIFT] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_color_shift(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_OBJECT_REMOVAL] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_object_removal(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_TEXTURE_CHANGE] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_texture_change(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_SHAPE_DEFORM] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_shape_deform(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_SUBTLE_PATTERN] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_subtle_pattern(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_SCALE_CHANGE] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_scale_change(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_ROTATION] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_rotation(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_FLIP] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_flip(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_BLUR] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_blur(img, region, difficulty, info);
    };
    diff_algorithms[DIFF_ADDITION] = [this](cv::Mat& img, const cv::Rect& region, int difficulty, DiffInfo& info) {
        this->apply_addition(img, region, difficulty, info);
    };
}

DiffGenerator::~DiffGenerator() {
    // 无需特殊清理
}

std::vector<cv::Rect> DiffGenerator::select_diff_regions(const cv::Mat& image, 
                                                        const std::vector<DetectedObject>& objects,
                                                        int count) {
    std::vector<cv::Rect> regions;
    
    // 如果检测到了对象，优先选择对象区域
    if (!objects.empty()) {
        // 复制所有检测到的边界框
        for (const auto& obj : objects) {
            regions.push_back(obj.bbox);
        }
        
        // 如果检测到的对象不够，添加随机区域
        if (regions.size() < static_cast<size_t>(count)) {
            std::uniform_int_distribution<int> distrib_w(0, image.cols - 50);
            std::uniform_int_distribution<int> distrib_h(0, image.rows - 50);
            std::uniform_int_distribution<int> distrib_size(30, 100);
            
            while (regions.size() < static_cast<size_t>(count)) {
                int x = distrib_w(rng);
                int y = distrib_h(rng);
                int size = distrib_size(rng);
                
                // 确保区域在图像内
                size = std::min(size, std::min(image.cols - x, image.rows - y));
                cv::Rect random_region(x, y, size, size);
                
                // 检查是否与已有区域重叠
                bool overlaps = false;
                for (const auto& existing : regions) {
                    if ((random_region & existing).area() > 0) {
                        overlaps = true;
                        break;
                    }
                }
                
                if (!overlaps) {
                    regions.push_back(random_region);
                }
            }
        }
    } else {
        // 如果没有检测到对象，创建随机区域
        std::uniform_int_distribution<int> distrib_w(0, image.cols - 50);
        std::uniform_int_distribution<int> distrib_h(0, image.rows - 50);
        std::uniform_int_distribution<int> distrib_size(30, 100);
        
        while (regions.size() < static_cast<size_t>(count)) {
            int x = distrib_w(rng);
            int y = distrib_h(rng);
            int size = distrib_size(rng);
            
            // 确保区域在图像内
            size = std::min(size, std::min(image.cols - x, image.rows - y));
            cv::Rect random_region(x, y, size, size);
            
            // 检查是否与已有区域重叠
            bool overlaps = false;
            for (const auto& existing : regions) {
                if ((random_region & existing).area() > 0) {
                    overlaps = true;
                    break;
                }
            }
            
            if (!overlaps) {
                regions.push_back(random_region);
            }
        }
    }
    
    // 如果区域太多，随机选择所需数量
    if (regions.size() > static_cast<size_t>(count)) {
        std::shuffle(regions.begin(), regions.end(), rng);
        regions.resize(count);
    }
    
    return regions;
}

bool DiffGenerator::generate_diffs(cv::Mat& image, const std::vector<DetectedObject>& objects, 
                                  int count, int difficulty, std::vector<DiffInfo>& diff_info) {
    // 参数验证
    if (image.empty()) {
        godot::UtilityFunctions::print_error("Empty image in generate_diffs");
        return false;
    }
    
    // 获取可以应用差异的区域
    std::vector<cv::Rect> regions = select_diff_regions(image, objects, count);
    
    if (regions.empty()) {
        godot::UtilityFunctions::print_error("No suitable regions found for differences");
        return false;
    }
    
    // 为每个区域应用差异
    for (const auto& region : regions) {
        DiffInfo info;
        
        // 选择适合难度的算法
        int algorithm_id = select_algorithm_for_difficulty(difficulty);
        
        // 应用差异
        apply_diff_algorithm(image, region, difficulty, info);
        
        // 设置差异信息
        info.algorithm_id = algorithm_id;
        info.position = Vector2(region.x + region.width/2, region.y + region.height/2);
        info.size = (region.width + region.height) / 2.0f;
        
        // 添加到结果
        diff_info.push_back(info);
    }
    
    return true;
}

int DiffGenerator::select_algorithm_for_difficulty(int difficulty) {
    // 根据难度选择算法
    // 难度越高，越倾向于选择更微妙的算法
    
    std::vector<int> algorithm_pool;
    
    if (difficulty <= 3) {
        // 简单难度 - 明显的变化
        algorithm_pool = {
            DIFF_COLOR_SHIFT,
            DIFF_OBJECT_REMOVAL,
            DIFF_ROTATION,
            DIFF_FLIP,
            DIFF_ADDITION
        };
    } else if (difficulty <= 7) {
        // 中等难度
        algorithm_pool = {
            DIFF_TEXTURE_CHANGE,
            DIFF_SHAPE_DEFORM,
            DIFF_SCALE_CHANGE,
            DIFF_BLUR
        };
    } else {
        // 高难度 - 微妙的变化
        algorithm_pool = {
            DIFF_SUBTLE_PATTERN,
            DIFF_SHAPE_DEFORM,
            DIFF_TEXTURE_CHANGE,
            DIFF_BLUR
        };
    }
    
    // 随机选择算法
    std::uniform_int_distribution<int> distrib(0, algorithm_pool.size() - 1);
    return algorithm_pool[distrib(rng)];
}

void DiffGenerator::apply_diff_algorithm(cv::Mat& image, const cv::Rect& region, int difficulty, 
                                       DiffInfo& diff_info) {
    // 选择算法
    int algorithm_id = select_algorithm_for_difficulty(difficulty);
    
    // 调用对应算法函数
    if (algorithm_id >= 0 && algorithm_id < static_cast<int>(diff_algorithms.size())) {
        diff_algorithms[algorithm_id](image, region, difficulty, diff_info);
        diff_info.algorithm_id = algorithm_id;
    } else {
        // 默认使用颜色变化
        apply_color_shift(image, region, difficulty, diff_info);
        diff_info.algorithm_id = DIFF_COLOR_SHIFT;
    }
}

// 差异算法实现

void DiffGenerator::apply_color_shift(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region);
    
    // 根据难度计算颜色变化强度
    float intensity = (11 - difficulty) * 2.5f;  // 难度越低，变化越明显
    
    // 随机选择颜色通道
    std::uniform_int_distribution<int> channel_dist(0, 2);
    int channel = channel_dist(rng);
    
    // 对选定通道应用变化
    for (int i = 0; i < roi.rows; i++) {
        for (int j = 0; j < roi.cols; j++) {
            cv::Vec3b& pixel = roi.at<cv::Vec3b>(i, j);
            
            // 应用颜色变化
            int new_value = pixel[channel] + intensity;
            new_value = std::max(0, std::min(255, new_value));
            pixel[channel] = static_cast<uchar>(new_value);
        }
    }
}

void DiffGenerator::apply_object_removal(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 从区域周围选择填充源
    cv::Point2i source;
    std::uniform_int_distribution<int> offset_dist(-100, 100);
    
    source.x = region.x + offset_dist(rng);
    source.y = region.y + offset_dist(rng);
    
    // 确保源在图像内
    source.x = std::max(0, std::min(image.cols - 1, source.x));
    source.y = std::max(0, std::min(image.rows - 1, source.y));
    
    // 创建填充遮罩
    cv::Mat mask = cv::Mat::zeros(region.height, region.width, CV_8UC1);
    cv::ellipse(mask, cv::Point(region.width/2, region.height/2), 
               cv::Size(region.width/2, region.height/2), 0, 0, 360, 
               cv::Scalar(255), -1);
    
    // 应用修复算法
    cv::Mat roi = image(region);
    cv::inpaint(roi, mask, roi, 3, cv::INPAINT_TELEA);
}

void DiffGenerator::apply_texture_change(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region);
    
    // 创建纹理掩码
    cv::Mat texture = cv::Mat::zeros(region.height, region.width, CV_8UC1);
    
    // 根据难度生成不同复杂度的纹理
    int pattern_size = 10 - difficulty / 2;  // 难度越高，纹理越细腻
    pattern_size = std::max(2, pattern_size);
    
    // 生成网格或随机点纹理
    for (int i = 0; i < roi.rows; i += pattern_size) {
        for (int j = 0; j < roi.cols; j += pattern_size) {
            cv::Rect pattern_rect(j, i, pattern_size, pattern_size);
            pattern_rect &= cv::Rect(0, 0, roi.cols, roi.rows); // 确保在ROI内
            
            if (pattern_rect.area() > 0) {
                std::uniform_int_distribution<int> pattern_dist(0, 255);
                cv::rectangle(texture, pattern_rect, cv::Scalar(pattern_dist(rng)), -1);
            }
        }
    }
    
    // 应用纹理变化
    float alpha = 0.2f; // 混合强度
    for (int i = 0; i < roi.rows; i++) {
        for (int j = 0; j < roi.cols; j++) {
            cv::Vec3b& pixel = roi.at<cv::Vec3b>(i, j);
            uchar tex_value = texture.at<uchar>(i, j);
            
            for (int c = 0; c < 3; c++) {
                float new_value = (1 - alpha) * pixel[c] + alpha * tex_value;
                pixel[c] = static_cast<uchar>(std::max(0.0f, std::min(255.0f, new_value)));
            }
        }
    }
}

void DiffGenerator::apply_shape_deform(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region).clone();
    
    // 计算变形参数
    float strength = (11 - difficulty) * 0.05f;  // 难度越低，变形越明显
    
    // 创建变形映射
    cv::Mat map_x(roi.size(), CV_32FC1);
    cv::Mat map_y(roi.size(), CV_32FC1);
    
    // 应用波浪变形
    for (int i = 0; i < roi.rows; i++) {
        for (int j = 0; j < roi.cols; j++) {
            // 计算扭曲映射
            map_x.at<float>(i, j) = j + strength * roi.cols * sin(i * M_PI / 30);
            map_y.at<float>(i, j) = i + strength * roi.rows * cos(j * M_PI / 30);
        }
    }
    
    // 应用变形
    cv::Mat deformed;
    cv::remap(roi, deformed, map_x, map_y, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    
    // 复制回原图
    deformed.copyTo(image(region));
}

void DiffGenerator::apply_subtle_pattern(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region);
    
    // 根据难度选择图案复杂度
    int pattern_complexity = difficulty;
    float intensity = 0.1f + (1.0f - difficulty / 10.0f) * 0.2f;  // 难度越高，强度越低
    
    // 创建一个随机模式
    std::uniform_int_distribution<int> pattern_dist(0, pattern_complexity);
    
    for (int i = 0; i < roi.rows; i++) {
        for (int j = 0; j < roi.cols; j++) {
            // 只修改满足模式的像素
            if ((i + j) % (pattern_complexity + 1) == pattern_dist(rng)) {
                cv::Vec3b& pixel = roi.at<cv::Vec3b>(i, j);
                
                for (int c = 0; c < 3; c++) {
                    float change = (pixel[c] < 128) ? intensity * 50 : -intensity * 50;
                    int new_value = pixel[c] + change;
                    pixel[c] = static_cast<uchar>(std::max(0, std::min(255, new_value)));
                }
            }
        }
    }
}

void DiffGenerator::apply_scale_change(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region).clone();
    
    // 根据难度计算缩放因子
    float scale_factor = 1.0f + (11 - difficulty) * 0.03f;
    if (std::uniform_int_distribution<int>(0, 1)(rng) == 0) {
        scale_factor = 1.0f / scale_factor;  // 有时缩小而不是放大
    }
    
    // 创建缩放后的图像
    cv::Mat scaled;
    cv::resize(roi, scaled, cv::Size(), scale_factor, scale_factor);
    
    // 裁剪或填充以适应原始区域
    cv::Mat result = cv::Mat::zeros(roi.size(), roi.type());
    
    cv::Rect src_rect(0, 0, 
                     std::min(scaled.cols, result.cols), 
                     std::min(scaled.rows, result.rows));
    cv::Rect dst_rect(0, 0, src_rect.width, src_rect.height);
    
    // 将缩放后的图像复制到结果中
    scaled(src_rect).copyTo(result(dst_rect));
    
    // 复制回原图
    result.copyTo(image(region));
}

void DiffGenerator::apply_rotation(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region).clone();
    
    // 根据难度计算旋转角度
    float angle = (11 - difficulty) * 3.0f;  // 难度越低，旋转越明显
    if (std::uniform_int_distribution<int>(0, 1)(rng) == 0) {
        angle = -angle;  // 随机方向
    }
    
    // 计算旋转中心
    cv::Point2f center(roi.cols / 2.0f, roi.rows / 2.0f);
    
    // 获取旋转矩阵
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, angle, 1.0);
    
    // 应用旋转
    cv::Mat rotated;
    cv::warpAffine(roi, rotated, rotation_matrix, roi.size());
    
    // 复制回原图
    rotated.copyTo(image(region));
}

void DiffGenerator::apply_flip(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region);
    
    // 根据难度选择翻转方式
    int flip_code;
    if (difficulty <= 3) {
        // 简单难度：水平或垂直翻转
        flip_code = std::uniform_int_distribution<int>(0, 1)(rng) ? 0 : 1;
    } else {
        // 更高难度：可能包括对角翻转
        flip_code = std::uniform_int_distribution<int>(-1, 1)(rng);
    }
    
    // 应用翻转
    cv::flip(roi, roi, flip_code);
}

void DiffGenerator::apply_blur(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region);
    
    // 根据难度计算模糊程度
    int kernel_size = std::max(3, 11 - difficulty);
    if (kernel_size % 2 == 0) kernel_size++; // 确保奇数
    
    // 应用高斯模糊
    cv::GaussianBlur(roi, roi, cv::Size(kernel_size, kernel_size), 0);
}

void DiffGenerator::apply_addition(cv::Mat& image, const cv::Rect& region, int difficulty, DiffInfo& diff_info) {
    // 提取区域
    cv::Mat roi = image(region);
    
    // 根据难度选择添加图案的复杂度
    int shape_type = std::uniform_int_distribution<int>(0, 2)(rng);
    cv::Scalar color;
    
    // 随机选择颜色
    for (int i = 0; i < 3; i++) {
        color[i] = std::uniform_int_distribution<int>(0, 255)(rng);
    }
    
    // 根据难度调整不透明度
    float alpha = 0.5f + (10 - difficulty) * 0.05f;  // 难度越高，越透明
    
    // 计算形状位置和大小
    int shape_size = region.width / 4;
    cv::Point center(roi.cols / 2, roi.rows / 2);
    
    // 创建遮罩
    cv::Mat shape_mask = cv::Mat::zeros(roi.size(), CV_8UC1);
    
    // 绘制形状
    switch (shape_type) {
        case 0: // 圆形
            cv::circle(shape_mask, center, shape_size, cv::Scalar(255), -1);
            break;
        case 1: // 矩形
            cv::rectangle(shape_mask, cv::Point(center.x - shape_size, center.y - shape_size), 
                         cv::Point(center.x + shape_size, center.y + shape_size), 
                         cv::Scalar(255), -1);
            break;
        case 2: // 三角形
            std::vector<cv::Point> triangle;
            triangle.push_back(cv::Point(center.x, center.y - shape_size));
            triangle.push_back(cv::Point(center.x - shape_size, center.y + shape_size));
            triangle.push_back(cv::Point(center.x + shape_size, center.y + shape_size));
            cv::fillPoly(shape_mask, std::vector<std::vector<cv::Point>>{triangle}, cv::Scalar(255));
            break;
    }
    
    // 将形状添加到ROI中
    for (int i = 0; i < roi.rows; i++) {
        for (int j = 0; j < roi.cols; j++) {
            if (shape_mask.at<uchar>(i, j) > 0) {
                cv::Vec3b& pixel = roi.at<cv::Vec3b>(i, j);
                for (int c = 0; c < 3; c++) {
                    pixel[c] = static_cast<uchar>(pixel[c] * (1 - alpha) + color[c] * alpha);
                }
            }
        }
    }
}

} // namespace godot 