#include "monot_esp/cam_capture_node.hpp"

CamCaptureNode::CamCaptureNode(const rclcpp::NodeOptions &options) : Node("cam_capture_node", options)
{
    RCLCPP_INFO(get_logger(), "Camera Capture Node Started.");

    img_pub_ = create_publisher<sensor_msgs::msg::Image>("camera_img", 10);
    timer_ = create_wall_timer(
        std::chrono::milliseconds(10), 
        std::bind(&CamCaptureNode::timer_callback, this)
    );

    RCLCPP_INFO(get_logger(), "Connecting to camera-server ...");
    cap_.open(url_);
    if (!cap_.isOpened()) {
      RCLCPP_ERROR(this->get_logger(), "Cannot open stream");
      rclcpp::shutdown();
      return;
    }
    RCLCPP_INFO(get_logger(), "Camera connected.");
}

void CamCaptureNode::timer_callback()
{
    cv::Mat frame;
    cap_ >> frame;
    cv::flip(frame, frame, 1);
    if(frame.empty())
    {
        RCLCPP_INFO(get_logger(), "Frame is empty.");
        return;
    }

    sensor_msgs::msg::Image msg_out;
    std_msgs::msg::Header header_;
    cv_bridge::CvImage cv_img;

    header_.stamp = get_clock() -> now();
    cv_img = cv_bridge::CvImage(header_, sensor_msgs::image_encodings::BGR8, frame);
    cv_img.toImageMsg(msg_out);
    img_pub_->publish(msg_out);
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(CamCaptureNode)