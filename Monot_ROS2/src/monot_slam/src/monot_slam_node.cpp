#include "monot_slam/monot_slam_node.hpp"
#include <rclcpp/qos.hpp>



MonotSlamNode::MonotSlamNode(const rclcpp::NodeOptions &options) : Node("monot_slam_node", options)
{
    RCLCPP_INFO(this->get_logger(), "Monot-SLAM-Node Started");

    set_parameters();

    if(SYSTEM_TYPE == "MONOCULAR")
    {
        slam_ = std::make_shared<ORB_SLAM3::System>(
            CNF_VOCAVULARY_PATH, CNF_SETTING_PATH, 
            ORB_SLAM3::System::MONOCULAR, true
        );
    }
    else if(SYSTEM_TYPE == "IMU_MONOCULAR")
    {
        slam_ = std::make_shared<ORB_SLAM3::System>(
            CNF_VOCAVULARY_PATH, CNF_SETTING_PATH, 
            ORB_SLAM3::System::IMU_MONOCULAR, true
        );
        imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
            "imu_data", rclcpp::SensorDataQoS(),
            std::bind(&MonotSlamNode::recieve_imu, this, std::placeholders::_1)
        );
    }
    else RCLCPP_FATAL(this->get_logger(), "Invalid system type: %s", SYSTEM_TYPE.c_str());

    img_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
        "camera_img", rclcpp::SensorDataQoS(),
        std::bind(&MonotSlamNode::recieve_image, this, std::placeholders::_1)
    );
}

MonotSlamNode::~MonotSlamNode()
{
    if(slam_) slam_->Shutdown();
}

void MonotSlamNode::set_parameters()
{
    this->declare_parameter<std::string>("system_type", SYSTEM_TYPE);
    this->declare_parameter<std::string>("vocabulary_path", CNF_VOCAVULARY_PATH);
    this->declare_parameter<std::string>("setting_path", CNF_SETTING_PATH);

    this->get_parameter("system_type", SYSTEM_TYPE);
    this->get_parameter("vocabulary_path", CNF_VOCAVULARY_PATH);
    this->get_parameter("setting_path", CNF_SETTING_PATH);

    RCLCPP_INFO(this->get_logger(), "System Type: %s", SYSTEM_TYPE.c_str());
    RCLCPP_INFO(this->get_logger(), "Vocabulary Path: %s", CNF_VOCAVULARY_PATH.c_str());
    RCLCPP_INFO(this->get_logger(), "Setting Path: %s", CNF_SETTING_PATH.c_str());
}

void MonotSlamNode::recieve_image(const sensor_msgs::msg::Image::SharedPtr msg)
{
    try
    {
        cv_img_ = cv_bridge::toCvCopy(msg);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return;
    }

    double timestamp = msg->header.stamp.sec + msg->header.stamp.nanosec * 1e-9;

    if(SYSTEM_TYPE == "MONOCULAR")
    {
        slam_->TrackMonocular(cv_img_->image, timestamp);
    }
    else if(SYSTEM_TYPE == "IMU_MONOCULAR")
    {
        std::vector<ORB_SLAM3::IMU::Point> imu_points;
        {
            std::lock_guard<std::mutex> lock(imu_mutex_);

            if(latest_imu_timestamp_ < timestamp + 0.01)
            {
                last_image_timestamp_ = timestamp;
                return;
            }
            
            while(!imu_buffer_.empty())
            {
                auto imu = imu_buffer_.front();

                if(imu.t <= last_image_timestamp_)
                {
                    imu_buffer_.pop_front();
                    continue;
                }

                if(imu.t <= timestamp)
                {
                    imu_points.push_back(imu);
                    imu_buffer_.pop_front();
                }
                else break;
            }
        }

        if(imu_points.empty()) return;

        slam_->TrackMonocular(cv_img_->image, timestamp, imu_points);
        last_image_timestamp_ = timestamp;
    }
}

void MonotSlamNode::recieve_imu(const sensor_msgs::msg::Imu::SharedPtr msg)
{
    double timestamp = msg->header.stamp.sec + msg->header.stamp.nanosec * 1e-9;

    ORB_SLAM3::IMU::Point imu_data(
        msg->angular_velocity.x, msg->angular_velocity.y, msg->angular_velocity.z,
        msg->linear_acceleration.x, msg->linear_acceleration.y, msg->linear_acceleration.z,
        timestamp
    );

    std::lock_guard<std::mutex> lock(imu_mutex_);
    imu_buffer_.push_back(imu_data);
    latest_imu_timestamp_ = timestamp;
    if(imu_buffer_.size() > IMU_BUFFER_SIZE) imu_buffer_.pop_front();
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(MonotSlamNode)