#ifndef MONOT_SLAM__MONOT_SLAM_NODE
#define MONOT_SLAM__MONOT_SLAM_NODE

#include <opencv2/core/core.hpp>

#include <rclcpp/rclcpp.hpp>
#include "cv_bridge/cv_bridge.h"

#include <System.h>

#include "sensor_msgs/msg/image.hpp"

#define CNF_VOCAVULARY_PATH "/home/racc_man/ORB-SLAM3/ORB_SLAM3/Vocabulary/ORBvoc.txt"
#define CNF_SETTING_PATH    "/home/racc_man/ORB-SLAM3/ORB_SLAM3/Examples/Monocular/RealSense_D435i.yaml"

class MonotSlamNode : public rclcpp::Node
{
    public:
        explicit MonotSlamNode(const rclcpp::NodeOptions & options);
        ~MonotSlamNode();

    private:
        void recieveImage(const sensor_msgs::msg::Image::SharedPtr msg);

        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_sub_;
        std::shared_ptr<ORB_SLAM3::System> slam_;
        cv_bridge::CvImagePtr cvImg_;
};

#endif