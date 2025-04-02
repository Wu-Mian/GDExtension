#include "yolo_detector.h"
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace godot {

YoloDetector::YoloDetector() 
    : is_initialized(false), 
      confidence_threshold(0.25f), 
      nms_threshold(0.45f) 
{
    // 在构造函数中初始化智能指针
    model = std::make_unique<litert::TFLiteModel>();
}

YoloDetector::~YoloDetector() {
    // 析构函数会自动处理释放资源
}

bool YoloDetector::initialize(const std::string& model_path) {
    // 已经初始化则返回true
    if (is_initialized) {
        return true;
    }
    
    try {
        // 加载YOLOv11模型
        if (!model->loadModel(model_path)) {
            return false;
        }
        
        // 设置模型参数
        model->setConfidenceThreshold(confidence_threshold);
        model->setNMSThreshold(nms_threshold);
        
        is_initialized = true;
        return true;
    } catch (const std::exception& e) {
        // 捕获并记录异常
        is_initialized = false;
        return false;
    }
}

bool YoloDetector::detect(const cv::Mat& image) {
    if (!is_initialized) {
        return false;
    }
    
    try {
        // 清除之前的检测结果
        detections.clear();
        
        // 执行模型推理
        if (!model->inference(image.data, image.total() * image.elemSize())) {
            return false;
        }
        
        // 获取检测结果
        auto results = model->getDetectionResults();
        
        // 将模型结果转换为DetectedObject结构
        for (const auto& result : results) {
            DetectedObject obj;
            obj.class_id = result.class_id;
            obj.confidence = result.confidence;
            obj.bounding_box = cv::Rect(
                result.x, 
                result.y, 
                result.width, 
                result.height
            );
            
            // 转换分割点
            obj.points.reserve(result.segmentation_points.size() / 2);
            for (size_t i = 0; i < result.segmentation_points.size(); i += 2) {
                obj.points.emplace_back(
                    result.segmentation_points[i], 
                    result.segmentation_points[i + 1]
                );
            }
            
            detections.push_back(obj);
        }
        
        return true;
    } catch (const std::exception& e) {
        // 捕获并记录异常
        return false;
    }
}

std::vector<DetectedObject> YoloDetector::get_detections() const {
    return detections;
}

void YoloDetector::set_confidence_threshold(float threshold) {
    confidence_threshold = threshold;
    if (is_initialized) {
        model->setConfidenceThreshold(threshold);
    }
}

float YoloDetector::get_confidence_threshold() const {
    return confidence_threshold;
}

void YoloDetector::set_nms_threshold(float threshold) {
    nms_threshold = threshold;
    if (is_initialized) {
        model->setNMSThreshold(threshold);
    }
}

float YoloDetector::get_nms_threshold() const {
    return nms_threshold;
}

void YoloDetector::draw_detections(cv::Mat& image) {
    // 在图像上绘制检测结果，用于调试
    for (const auto& det : detections) {
        // 绘制边界框
        cv::rectangle(image, det.bounding_box, cv::Scalar(0, 255, 0), 2);
        
        // 绘制分割轮廓
        std::vector<std::vector<cv::Point>> contours;
        contours.push_back(det.points);
        cv::drawContours(image, contours, 0, cv::Scalar(0, 0, 255), 2);
        
        // 绘制置信度文字
        std::string text = "Class " + std::to_string(det.class_id) + 
                          " (" + std::to_string(int(det.confidence * 100)) + "%)";
        cv::putText(image, text, 
                   cv::Point(det.bounding_box.x, det.bounding_box.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    }
}

} // namespace godot 