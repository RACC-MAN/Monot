#ifndef MONOT_SLAM__MONOT_SLAM_NODE
#define MONOT_SLAM__MONOT_SLAM_NODE

#include <opencv2/core/core.hpp>

#include <rclcpp/rclcpp.hpp>
#include "cv_bridge/cv_bridge.h"

#include <System.h>

#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"

class MonotSlamNode : public rclcpp::Node
{
    public:
        explicit MonotSlamNode(const rclcpp::NodeOptions & options);
        ~MonotSlamNode();

    private:
        void recieve_image(const sensor_msgs::msg::Image::SharedPtr msg);
        void recieve_imu(const sensor_msgs::msg::Imu::SharedPtr msg);
        void set_parameters();

        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_sub_;
        cv_bridge::CvImagePtr cv_img_;
        double last_image_timestamp_ = -1.0;

        rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
        double latest_imu_timestamp_ = -1.0;
        std::deque<ORB_SLAM3::IMU::Point> imu_buffer_;
        size_t IMU_BUFFER_SIZE = 400;
        std::mutex imu_mutex_;

        std::shared_ptr<ORB_SLAM3::System> slam_;

        std::string SYSTEM_TYPE = "MONOCULAR";
        std::string CNF_VOCAVULARY_PATH = "/path/to/ORBvoc.txt";
        std::string CNF_SETTING_PATH = "/path/to/camera_params.yaml";
};

#endif