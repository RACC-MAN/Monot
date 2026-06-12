#include "monot_bringup/udp_image_reciever.hpp"

UdpImageReciever::UdpImageReciever(const rclcpp::NodeOptions &options) : Node("udp_image_reciever", options)
{
    RCLCPP_INFO(get_logger(), "UDP Image Reciever Node Started.");

    img_pub_ = create_publisher<sensor_msgs::msg::Image>("camera_img", 10);

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        RCLCPP_ERROR(get_logger(), "Failed to bind socket.");
        throw std::runtime_error("Failed to bind socket.");
    }

    recv_thread_ = std::thread(&UdpImageReciever::receive_loop, this);
}

UdpImageReciever::~UdpImageReciever()
{
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
}

void UdpImageReciever::receive_loop()
{
    uint8_t buffer[BUFFER_SIZE];
    
    sockaddr_in sender_addr{};
    socklen_t sender_len = sizeof(sender_addr);

    while(rclcpp::ok()) {
        int len = recvfrom(sock_, buffer, BUFFER_SIZE, 0, (sockaddr*)&sender_addr, &sender_len);
        if (len < 6) continue; // Not enough data for header

        uint16_t frame_id = ntohs(*(uint16_t*)(buffer));
        uint16_t packet_id = ntohs(*(uint16_t*)(buffer + 2));
        uint16_t total_packets = ntohs(*(uint16_t*)(buffer + 4));

        auto &frame = frames_[frame_id];

        if (frame.packets.empty()) {
            frame.total_packets = total_packets;
            frame.packets.resize(total_packets);
        }

        frame.packets[packet_id] = std::vector<uint8_t>(buffer + 6, buffer + len);

        bool complete = true;
        for (const auto &pkt : frame.packets) {
            if (pkt.empty()) {
                complete = false;
                // RCLCPP_WARN(get_logger(), "Frame %u is incomplete, waiting for more packets...", frame_id);
                break;
            }
        }

        if(complete) {
            RCLCPP_INFO(get_logger(), "Received complete frame %u with %u packets, assembling image...", frame_id, total_packets);
            std::vector<uint8_t> img_data;
            for (const auto &pkt : frame.packets) {
                img_data.insert(img_data.end(), pkt.begin(), pkt.end());
            }

            cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);
            if (!img.empty()) 
            {
                if(flip_image) cv::flip(img, img, 1);
                if(SCALE_SIZE != 1) cv::resize(img, img, cv::Size(), SCALE_SIZE, SCALE_SIZE, cv::INTER_LINEAR);

                auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", img).toImageMsg();
                img_pub_->publish(*msg);
                RCLCPP_INFO(get_logger(), "Published complete image for frame %u, image size: %u x %u", frame_id, img.rows, img.cols);
            } 
            else RCLCPP_ERROR(get_logger(), "Failed to decode image for frame %u", frame_id);

            frames_.erase(frame_id);
        }

        if(frames_.size() > 10) {
            RCLCPP_INFO(get_logger(), "Too many incomplete frames, clearing buffer.");
            frames_.clear();
        }
    }
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(UdpImageReciever)