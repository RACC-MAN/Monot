#include "monot_slam/monot_slam_node.hpp"

MonotSlamNode::MonotSlamNode(const rclcpp::NodeOptions &options) : Node("monot_slam_node", options)
{
    RCLCPP_INFO(this->get_logger(), "Monot-SLAM-Node Started");

    slam_ = std::make_shared<ORB_SLAM3::System>(
        CNF_VOCAVULARY_PATH, CNF_SETTING_PATH, 
        ORB_SLAM3::System::MONOCULAR, true
    );

    img_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
        "/image_raw", 10,
        std::bind(&MonotSlamNode::recieveImage, this, std::placeholders::_1)
    );
}

MonotSlamNode::~MonotSlamNode()
{
}

void MonotSlamNode::recieveImage(const sensor_msgs::msg::Image::SharedPtr msg)
{
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(MonotSlamNode)