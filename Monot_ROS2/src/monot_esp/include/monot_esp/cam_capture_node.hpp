#ifndef MONOT_ESP__CAM_CAPTURE_NODE
#define MONOT_ESP__CAM_CAPTURE_NODE

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

class CamCaptureNode : public rclcpp::Node
{
    public :
        explicit CamCaptureNode(const rclcpp::NodeOptions &options);
    
    private:
        void timer_callback();

        cv::VideoCapture cap_;

        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr img_pub_;
        rclcpp::TimerBase::SharedPtr timer_;

        std::string url_ = "http://111.111.11.11";
};

#endif