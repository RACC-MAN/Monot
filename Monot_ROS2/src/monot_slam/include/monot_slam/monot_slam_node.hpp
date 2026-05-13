#ifndef MONOT_SLAM__MONOT_SLAM_NODE
#define MONOT_SLAM__MONOT_SLAM_NODE

#include <opencv2/core/core.hpp>

#include <rclcpp/rclcpp.hpp>
#include "cv_bridge/cv_bridge.h"

#include <System.h>

#include "sensor_msgs/msg/image.hpp"

#define CNF_VOCAVULARY_PATH "/home/keisoku/ORB-SLAM3/ORB_SLAM3/Vocabulary/ORBvoc.txt"

// Path to the settings file for the camera. This should be a YAML file containing the camera intrinsics and distortion coefficients.
// #define CNF_SETTING_PATH    "/home/keisoku/monot_ws/Monot/camera_calibration/webcam_320x240_param.yaml"
// #define CNF_SETTING_PATH    "/home/keisoku/monot_ws/Monot/camera_calibration/camera_ov3660_param.yaml"
#define CNF_SETTING_PATH    "/home/keisoku/monot_ws/Monot/camera_calibration/udp_640x480_param.yaml"
// #define CNF_SETTING_PATH    "/home/keisoku/monot_ws/Monot/camera_calibration/udp_1280x960_param.yaml"

class MonotSlamNode : public rclcpp::Node
{
    public:
        explicit MonotSlamNode(const rclcpp::NodeOptions & options);
        ~MonotSlamNode();

    private:
        void recieve_image(const sensor_msgs::msg::Image::SharedPtr msg);

        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_sub_;
        std::shared_ptr<ORB_SLAM3::System> slam_;
        cv_bridge::CvImagePtr cv_img_;
};

#endif