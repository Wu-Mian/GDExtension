#ifndef YOLO_DETECTOR_H
#define YOLO_DETECTOR_H

#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <litert/tflite_model.h>

/**
 * 表示检测到的物体的结构体
 */
struct DetectedObject {
    int class_id;                  // 类别ID
    float confidence;              // 置信度
    cv::Rect bounding_box;         // 边界框
    std::vector<cv::Point> points; // 分割点集
};

/**
 * YOLOv11检测器类
 * 负责加载并运行YOLO模型，生成检测结果
 */
class YoloDetector {
public:
    YoloDetector();
    ~YoloDetector();

    /**
     * 初始化检测器
     * @param model_path YOLO模型文件路径
     * @return 成功返回true，失败返回false
     */
    bool initialize(const std::string& model_path);

    /**
     * 在图像上运行检测
     * @param image 待检测的图像
     * @return 成功返回true，失败返回false
     */
    bool detect(const cv::Mat& image);

    /**
     * 获取检测结果
     * @return 检测到的物体列表
     */
    std::vector<DetectedObject> get_detections() const;

    /**
     * 设置置信度阈值
     * @param threshold 新的置信度阈值
     */
    void set_confidence_threshold(float threshold);

    /**
     * 获取当前置信度阈值
     * @return 当前置信度阈值
     */
    float get_confidence_threshold() const;

    /**
     * 设置NMS阈值
     * @param threshold 新的NMS阈值
     */
    void set_nms_threshold(float threshold);

    /**
     * 获取当前NMS阈值
     * @return 当前NMS阈值
     */
    float get_nms_threshold() const;

    /**
     * 在图像上绘制检测结果
     * @param image 要绘制的图像
     */
    void draw_detections(cv::Mat& image);

private:
    bool is_initialized;               // 是否已初始化
    float confidence_threshold;        // 置信度阈值
    float nms_threshold;               // 非极大值抑制阈值
    std::vector<DetectedObject> detections; // 检测结果
    std::unique_ptr<litert::TFLiteModel> model; // TFLite模型
};

#endif // YOLO_DETECTOR_H 