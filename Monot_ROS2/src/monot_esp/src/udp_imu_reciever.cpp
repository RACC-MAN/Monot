#include "monot_esp/udp_imu_reciever.hpp"

UdpImuReciever::UdpImuReciever(const rclcpp::NodeOptions &options) : Node("udp_imu_reciever", options)
{
    RCLCPP_INFO(get_logger(), "UDP IMU Reciever Node Started.");

    imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("imu_data", 10);

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        RCLCPP_ERROR(get_logger(), "Failed to bind socket.");
        throw std::runtime_error("Failed to bind socket.");
    }

    recv_thread_ = std::thread(&UdpImuReciever::receive_loop, this);
}

UdpImuReciever::~UdpImuReciever()
{
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
}

void UdpImuReciever::receive_loop()
{
    uint8_t buffer[1024];
    
    sockaddr_in sender_addr{};
    socklen_t sender_len = sizeof(sender_addr);

    while(rclcpp::ok()) {
        int len = recvfrom(sock_, buffer, sizeof(buffer), 0, (sockaddr*)&sender_addr, &sender_len);
        if (len < sizeof(ImuPacket)) continue;

        ImuPacket *pkt = (ImuPacket*)buffer;

        auto imu_msg = std::make_shared<sensor_msgs::msg::Imu>();
        imu_msg->header.stamp = this->now();
        imu_msg->linear_acceleration.x = pkt->ax;
        imu_msg->linear_acceleration.y = pkt->ay;
        imu_msg->linear_acceleration.z = pkt->az;
        imu_msg->angular_velocity.x = pkt->gx;
        imu_msg->angular_velocity.y = pkt->gy;
        imu_msg->angular_velocity.z = pkt->gz;

        imu_pub_->publish(*imu_msg);
        RCLCPP_INFO(get_logger(), "Published IMU data");
    }
}


#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(UdpImuReciever)