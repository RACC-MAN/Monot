#ifndef MONOT_BRINGUP__UDP_IMU_RECIEVER
#define MONOT_BRINGUP__UDP_IMU_RECIEVER

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>

#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <vector>

#define PORT 9001

struct ImuPacket
{
    uint64_t timestamp_us;
    float ax, ay, az; // Accelerometer
    float gx, gy, gz; // Gyroscope
};

class UdpImuReciever : public rclcpp::Node
{
    public :
        explicit UdpImuReciever(const rclcpp::NodeOptions &options);
        ~UdpImuReciever();
    
    private:
        void receive_loop();

        int sock_;
        std::thread recv_thread_;
        rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
};

#endif