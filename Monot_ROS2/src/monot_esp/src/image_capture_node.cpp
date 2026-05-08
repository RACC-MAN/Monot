#include "monot_esp/image_capture_node.hpp"

ImageCaptureNode::ImageCaptureNode(const rclcpp::NodeOptions &options) : Node("image_capture_node", options)
{
    RCLCPP_INFO(get_logger(), "Image Capture Node Started.");

    get_parameter("save_method", save_method_);
    RCLCPP_INFO(get_logger(),"save_method: %d", save_method_);

    img_sub_ = create_subscription<sensor_msgs::msg::Image>(
        "camera_img", 10, std::bind(&ImageCaptureNode::image_callback, this, std::placeholders::_1)
    );

    switch (save_method_)
    {
        case 0:
            RCLCPP_INFO(get_logger(), "Method : TIMER");
            timer_ = create_wall_timer(
                std::chrono::milliseconds(1000), 
                std::bind(&ImageCaptureNode::timer_callback, this)
            );
            break;

        case 1:
            RCLCPP_INFO(get_logger(), "Method : LOOP");
            recv_thread_ = std::thread(&ImageCaptureNode::loop, this);
            break;
        
        default:
            break;
    }

    capture_once_ = false;
    frame_count_ = 0;
}

ImageCaptureNode::~ImageCaptureNode()
{
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
}

void ImageCaptureNode::image_callback(sensor_msgs::msg::Image::UniquePtr msg)
{
    capture_once_ = true;
    latest_image_ = std::move(msg);
}

void ImageCaptureNode::timer_callback()
{
    if (!capture_once_) {
        RCLCPP_WARN(get_logger(), "No image received yet.");
        return;
    }

    capture_once_ = false;

    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(*latest_image_, sensor_msgs::image_encodings::BGR8);
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    cv::Mat img = cv_ptr->image;
    if (img.empty()) {
        RCLCPP_WARN(get_logger(), "Received empty image.");
        return;
    }

    std::string filename = save_directory_ + "image_" + std::to_string(frame_count_) + ".jpg";
    cv::imwrite(filename, img);
    RCLCPP_INFO(get_logger(), "Saved %s", filename.c_str());
    frame_count_++; 
}

void ImageCaptureNode::loop()
{
    std::string input;

    while(rclcpp::ok()) {

        std::cin >> input;
        if (true) {
            if(!capture_once_) {
                RCLCPP_WARN(get_logger(), "No image received yet.");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return;
            }

            capture_once_ = false;

            cv_bridge::CvImagePtr cv_ptr;
            try {
                cv_ptr = cv_bridge::toCvCopy(*latest_image_, sensor_msgs::image_encodings::BGR8);
            } catch (cv_bridge::Exception& e) {
                RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
                return;
            }

            cv::Mat img = cv_ptr->image;
            if (img.empty()) {
                RCLCPP_WARN(get_logger(), "Received empty image.");
                return;
            }


            std::string filename = save_directory_ + "image_" + std::to_string(frame_count_) + ".jpg";
            cv::imwrite(filename, img);
            RCLCPP_INFO(get_logger(), "Saved %s", filename.c_str());
            frame_count_++; 
        }
    }
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(ImageCaptureNode)