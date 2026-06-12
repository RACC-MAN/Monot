#ifndef MONOT_BRINGUP__UDP_IMAGE_RECIEVER
#define MONOT_BRINGUP__UDP_IMAGE_RECIEVER

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <vector>

#define PORT 9000
#define BUFFER_SIZE 1500

const int SCALE_SIZE = 2;
const bool flip_image = false;

struct FrameBuffer {
    std::vector<std::vector<uint8_t>> packets;
    int total_packets;
};

class UdpImageReciever : public rclcpp::Node
{
    public :
        explicit UdpImageReciever(const rclcpp::NodeOptions &options);
        ~UdpImageReciever();
    
    private:
        void receive_loop();

        int sock_;
        std::thread recv_thread_;
        std::map<uint16_t, FrameBuffer> frames_;
        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr img_pub_;
};

#endif