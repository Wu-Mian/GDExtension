#include "yolo_detector.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <opencv2/imgproc.hpp>
#include <litert/nn/tflite/yolov5.h>

namespace godot {

YoloDetector::YoloDetector() 
    : is_initialized(false),
      confidence_threshold(0.5f),
      nms_threshold(0.45f) {
}

YoloDetector::~YoloDetector() {
    // 智能指针会自动清理
}

bool YoloDetector::initialize(const std::string& model_file) {
    try {
        // 初始化YOLOv11模型 (使用YOLOv5接口，因为LiteRT内部处理兼容)
        yolo_detector = std::make_unique<lite::nn::tflite::YoloV5>(model_file);
        
        // 设置阈值参数
        yolo_detector->set_conf_thresh(confidence_threshold);
        yolo_detector->set_nms_thresh(nms_threshold);
        
        is_initialized = true;
        return true;
    } catch (const std::exception& e) {
        godot::UtilityFunctions::print_error("YoloDetector initialization failed: ", e.what());
        is_initialized = false;
        return false;
    }
}

bool YoloDetector::detect(const cv::Mat& image) {
    if (!is_initialized) {
        godot::UtilityFunctions::print_error("YoloDetector not initialized");
        return false;
    }
    
    try {
        // 清除上一次的检测结果
        last_detections.clear();
        
        // 运行推理
        auto infer_results = yolo_detector->detect(image);
        
        // 转换结果格式为我们的DetectedObject结构
        for (const auto& res : infer_results) {
            DetectedObject obj;
            obj.class_id = res.label_id;
            obj.confidence = res.confidence;
            
            // 边界框
            obj.bbox = cv::Rect(res.bbox.x, res.bbox.y, res.bbox.width, res.bbox.height);
            
            // 分割点
            if (!res.segmentation.points.empty()) {
                for (const auto& point : res.segmentation.points) {
                    obj.segment.push_back(cv::Point(point.x, point.y));
                }
            }
            
            last_detections.push_back(obj);
        }
        
        return true;
    } catch (const std::exception& e) {
        godot::UtilityFunctions::print_error("YoloDetector detection failed: ", e.what());
        return false;
    }
}

const std::vector<DetectedObject>& YoloDetector::get_detections() const {
    return last_detections;
}

void YoloDetector::set_confidence_threshold(float threshold) {
    confidence_threshold = threshold;
    if (is_initialized) {
        yolo_detector->set_conf_thresh(confidence_threshold);
    }
}

float YoloDetector::get_confidence_threshold() const {
    return confidence_threshold;
}

void YoloDetector::set_nms_threshold(float threshold) {
    nms_threshold = threshold;
    if (is_initialized) {
        yolo_detector->set_nms_thresh(nms_threshold);
    }
}

float YoloDetector::get_nms_threshold() const {
    return nms_threshold;
}

void YoloDetector::draw_detections(cv::Mat& image) {
    if (last_detections.empty()) {
        return;
    }
    
    for (const auto& obj : last_detections) {
        // 画边界框
        cv::rectangle(image, obj.bbox, cv::Scalar(0, 255, 0), 2);
        
        // 画分割区域
        if (obj.segment.size() > 2) {
            std::vector<std::vector<cv::Point>> contours;
            contours.push_back(obj.segment);
            cv::drawContours(image, contours, 0, cv::Scalar(0, 0, 255), 2);
        }
        
        // 绘制置信度文本
        std::string label = "Conf: " + std::to_string(obj.confidence).substr(0, 4);
        int font_face = cv::FONT_HERSHEY_SIMPLEX;
        double font_scale = 0.5;
        int thickness = 1;
        cv::Size text_size = cv::getTextSize(label, font_face, font_scale, thickness, 0);
        cv::Point text_org(obj.bbox.x, obj.bbox.y - 5);
        
        cv::rectangle(image, cv::Rect(text_org.x, text_org.y - text_size.height, 
                                     text_size.width, text_size.height + 5),
                     cv::Scalar(0, 255, 0), -1);
        cv::putText(image, label, text_org, font_face, font_scale, 
                   cv::Scalar(0, 0, 0), thickness);
    }
}

} // namespace godot 