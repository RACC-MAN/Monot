#include "monot_slam/monot_slam_node.hpp"

MonotSlamNode::MonotSlamNode(const rclcpp::NodeOptions &options) : Node("monot_slam_node", options)
{
    RCLCPP_INFO(this->get_logger(), "Monot-SLAM-Node Started");

    slam_ = std::make_shared<ORB_SLAM3::System>(
        CNF_VOCAVULARY_PATH, CNF_SETTING_PATH, 
        ORB_SLAM3::System::MONOCULAR, true
    );

    img_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
        "/camera_img", 10,
        std::bind(&MonotSlamNode::recieve_image, this, std::placeholders::_1)
    );
}

MonotSlamNode::~MonotSlamNode()
{
}

void MonotSlamNode::recieve_image(const sensor_msgs::msg::Image::SharedPtr msg)
{
    // RCLCPP_INFO(this->get_logger(), "Image received.");
    try
    {
        cv_img_ = cv_bridge::toCvCopy(msg);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return;
    }
    
    slam_->TrackMonocular(cv_img_->image, msg->header.stamp.sec);
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(MonotSlamNode)