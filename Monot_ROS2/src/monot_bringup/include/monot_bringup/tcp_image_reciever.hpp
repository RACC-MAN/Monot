#ifndef MONOT_BRINGUP__TCP_IMAGE_RECIEVER
#define MONOT_BRINGUP__TCP_IMAGE_RECIEVER

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

class TcpImageReciever : public rclcpp::Node
{
    public :
        explicit TcpImageReciever(const rclcpp::NodeOptions &options);
        ~TcpImageReciever();
    
    private:
        void receive_loop();
        int receive_image(uint8_t* data, size_t length);

        std::thread recv_thread_;
        int server_fd_, client_fd_;
        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr img_pub_;
        rclcpp::TimerBase::SharedPtr timer_;

        int frame_count_ = 0;
};

#endif