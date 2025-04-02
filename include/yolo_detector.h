#ifndef YOLO_DETECTOR_H
#define YOLO_DETECTOR_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>

// LiteRT 前向声明
namespace lite {
    namespace nn {
        namespace tflite {
            class YoloV5;
        }
    }
}

namespace godot {

// 检测到的对象信息
struct DetectedObject {
    int class_id;
    float confidence;
    cv::Rect bbox;
    std::vector<cv::Point> segment; // 分割点
};

class YoloDetector {
private:
    std::unique_ptr<lite::nn::tflite::YoloV5> yolo_detector;
    std::string model_path;
    bool is_initialized;
    
    // 模型配置参数
    float confidence_threshold;
    float nms_threshold;
    
    std::vector<DetectedObject> last_detections;

public:
    YoloDetector();
    ~YoloDetector();
    
    // 初始化检测器
    bool initialize(const std::string& model_path);
    
    // 运行检测
    bool detect(const cv::Mat& image);
    
    // 获取检测结果
    const std::vector<DetectedObject>& get_detections() const;
    
    // 设置/获取阈值
    void set_confidence_threshold(float threshold);
    float get_confidence_threshold() const;
    
    void set_nms_threshold(float threshold);
    float get_nms_threshold() const;
    
    // 绘制检测结果到图像上（调试用）
    void draw_detections(cv::Mat& image);
};

} // namespace godot

#endif // YOLO_DETECTOR_H 