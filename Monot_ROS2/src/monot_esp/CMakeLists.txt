cmake_minimum_required(VERSION 3.8)
project(monot_esp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()


# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/cam_capture_node.cpp
  src/image_capture_node.cpp
  src/tcp_image_reciever.cpp
  src/udp_image_reciever.cpp
) 

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "CamCaptureNode"
  EXECUTABLE cam_capture_node
)

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "ImageCaptureNode"
  EXECUTABLE image_capture_node
)

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "TcpImageReciever"
  EXECUTABLE tcp_image_reciever
)

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "UdpImageReciever"
  EXECUTABLE udp_image_reciever
)

target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package(
  INSTALL_TO_SHARE
)

install(FILES DESTINATION share/${PROJECT_NAME})