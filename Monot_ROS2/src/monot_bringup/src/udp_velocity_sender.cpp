#include "monot_bringup/udp_velocity_sender.hpp"

UdpVelocitySender::UdpVelocitySender(const rclcpp::NodeOptions &options) : Node("udp_velocity_sender", options)
{
    RCLCPP_INFO(get_logger(), "UDP Velocity Sender Node Started.");

    // Load parameters
    declare_parameter("esp32_ip", "192.168.11.8");
    declare_parameter("port", 9001);

    declare_parameter("input_type", "twist");

    declare_parameter("max_linear_speed", 1.0);
    declare_parameter("max_angular_speed", 1.0);

    // Set parameters
    get_parameter("esp32_ip", esp_ip_);
    get_parameter("port", port_);

    get_parameter("input_type", input_type_);

    get_parameter("max_linear_speed", max_linear_);
    get_parameter("max_angular_speed", max_angular_);


    // Setup
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port_);
    inet_pton(AF_INET, esp_ip_.c_str(), &addr_.sin_addr);

    if(input_type_ == "joy")
    {
        joy_sub_ = create_subscription<sensor_msgs::msg::Joy>(
            "/joy", 10, std::bind(&UdpVelocitySender::joyCallback, this, std::placeholders::_1));
    }
    else if(input_type_ == "twist")
    {
        twist_sub_ = create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10, std::bind(&UdpVelocitySender::twistCallback, this, std::placeholders::_1));
    }
    else
    {
        RCLCPP_ERROR(get_logger(), "Invalid input type: %s", input_type_.c_str());
    }

    RCLCPP_INFO(get_logger(), "Start Joy UDP Sender");
}

UdpVelocitySender::~UdpVelocitySender()
{
    close(sock_);
}

void UdpVelocitySender::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg)
{
    if(msg->axes.size() < 4)
        return;

    VelocityCommand cmd;
    cmd.linear = msg->axes[3] * max_linear_;
    cmd.angular = msg->axes[2] * max_angular_;

    sendto( sock_, &cmd, sizeof(cmd), 0, (sockaddr*)&addr_, sizeof(addr_));
    RCLCPP_INFO(get_logger(), "Sent Velocity Command: linear=%.2f, angular=%.2f", cmd.linear, cmd.angular);
}

void UdpVelocitySender::twistCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    VelocityCommand cmd;
    cmd.linear = msg->linear.x * max_linear_;
    cmd.angular = msg->angular.z * max_angular_;

    sendto( sock_, &cmd, sizeof(cmd), 0, (sockaddr*)&addr_, sizeof(addr_));
    RCLCPP_INFO(get_logger(), "Sent Velocity Command: linear=%.2f, angular=%.2f", cmd.linear, cmd.angular);
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(UdpVelocitySender)