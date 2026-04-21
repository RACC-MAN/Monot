#include "monot_esp/tcp_image_reciever.hpp"
TcpImageReciever::TcpImageReciever(const rclcpp::NodeOptions &options) : Node("tcp_image_reciever", options)
{
    RCLCPP_INFO(get_logger(), "TCP Image Reciever Node Started.");

    img_pub_ = create_publisher<sensor_msgs::msg::Image>("camera_img", 10);

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    bind(server_fd_, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd_, 1);

    RCLCPP_INFO(get_logger(), "Waiting for client connection ...");
    client_fd_ = accept(server_fd_, nullptr, nullptr);
    RCLCPP_INFO(get_logger(), "Client connected.");

    recv_thread_ = std::thread(&TcpImageReciever::receive_loop, this);
    // timer_ = create_wall_timer(std::chrono::milliseconds(20), std::bind(&TcpImageReciever::receive_loop, this));
}

void TcpImageReciever::receive_loop()
{
    while(rclcpp::ok()) {
        uint32_t size;
        int ret = receive_image((uint8_t*)&size, 4);
        size = ntohl(size);
        if (ret <= 0) {
            RCLCPP_ERROR(get_logger(), "Connection closed.");
            return;
        }

        if (size == 0 || size > 200000) {
            RCLCPP_ERROR(get_logger(), "Invalid size: %u", size);
            return;
        }

        std::vector<uint8_t> buffer(size);

        ret = receive_image(buffer.data(), size);
        if (ret <= 0) {
            RCLCPP_ERROR(get_logger(), "Failed to receive image data.");
            return;
        }

        if (buffer[0] != 0xFF || buffer[1] != 0xD8) {
            std::cout << "Invalid JPEG header, skip" << std::endl;
            return;
        }

        cv::Mat img = cv::imdecode(buffer, cv::IMREAD_COLOR);
        if (img.empty()) {
            std::cout << "Decode failed, skipping frame" << std::endl;
            return;
        }

        auto msg = cv_bridge::CvImage(
            std_msgs::msg::Header(),
            "bgr8",
            img
        ).toImageMsg();

        img_pub_->publish(*msg);
        // std::cout << "Image published, size: " << img.cols << "x" << img.rows << std::endl;
    }
}

int TcpImageReciever::receive_image(uint8_t* data, size_t length)
{
    // std::cout << "Receiving image..." << std::endl;
    size_t total_received = 0;
    while (total_received < length) {
    // std::cout << "Total received: " << total_received << " / " << length << std::endl;
        ssize_t received = recv(client_fd_, data + total_received, length - total_received, 0);
        if (received <= 0) return received; // Error or connection closed
        total_received += received;
    }
    return total_received; // Success
}

TcpImageReciever::~TcpImageReciever()
{
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
}


#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(TcpImageReciever)