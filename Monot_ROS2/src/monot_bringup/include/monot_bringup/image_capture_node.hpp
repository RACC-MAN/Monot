#ifndef MONOT_BRINGUP__IMAGE_CAPTURE_NODE
#define MONOT_BRINGUP__IMAGE_CAPTURE_NODE

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

class ImageCaptureNode : public rclcpp::Node
{
    public :
        explicit ImageCaptureNode(const rclcpp::NodeOptions &options);
        ~ImageCaptureNode();
    
    private:
        void image_callback(sensor_msgs::msg::Image::UniquePtr msg);
        void timer_callback();
        void loop();

        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_sub_;
        rclcpp::TimerBase::SharedPtr timer_;

        sensor_msgs::msg::Image::UniquePtr latest_image_;
        bool capture_once_ = false;
        std::thread recv_thread_;

        int frame_count_ = 0;
        std::string save_directory_ = "./captured_images/";
        int save_method_ = 1; // 0: timer, 1: on demand
};

#endif