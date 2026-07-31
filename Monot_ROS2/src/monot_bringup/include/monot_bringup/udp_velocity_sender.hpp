#ifndef MONOT_BRINGUP__UDP_VELOCITY_SENDER
#define MONOT_BRINGUP__UDP_VELOCITY_SENDER

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

struct VelocityCommand
{
    float linear;
    float angular;
};

class UdpVelocitySender : public rclcpp::Node
{
    public :
        explicit UdpVelocitySender(const rclcpp::NodeOptions &options);
        ~UdpVelocitySender();
    
    private:
        void joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg);
        void twistCallback(const geometry_msgs::msg::Twist::SharedPtr msg);

        rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
        rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr twist_sub_;

        int sock_;
        sockaddr_in addr_;

        std::string esp_ip_;
        int port_;

        std::string input_type_;

        double max_linear_;
        double max_angular_;
};

#endif