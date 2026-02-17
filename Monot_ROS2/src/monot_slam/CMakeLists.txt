cmake_minimum_required(VERSION 3.8)
project(monot_slam)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic
    -Wno-deprecated -Wno-reorder -Wno-unused-parameter
    -Wno-deprecated-declarations -Wno-sign-compare -Wno-pedantic
    -Wno-overloaded-virtual
    )
endif()

# -------------------------
# ROS2 dependencies
# -------------------------
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)

# -------------------------
# ORB-SLAM3 external path
# -------------------------
set(ORB_SLAM3_ROOT "/home/keisoku/ORB-SLAM3/ORB_SLAM3")

# -------------------------
# Library target
# -------------------------
add_library(${PROJECT_NAME} SHARED
  src/monot_slam_node.cpp
)

# include AFTER add_library
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${ORB_SLAM3_ROOT}/include
    ${ORB_SLAM3_ROOT}/include/CameraModels
    ${ORB_SLAM3_ROOT}/Thirdparty/Sophus
    ${ORB_SLAM3_ROOT}
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
)

# link AFTER add_library
target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  ${EIGEN3_LIBS}
  ${Pangolin_LIBRARIES}
  ${ORB_SLAM3_ROOT}/lib/libORB_SLAM3.so
  ${ORB_SLAM3_ROOT}/Thirdparty/DBoW2/lib/libDBoW2.so
  ${ORB_SLAM3_ROOT}/Thirdparty/g2o/lib/libg2o.so
  -lboost_serialization
  -lcrypto
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  rclcpp_components
  std_msgs
  sensor_msgs
  cv_bridge
)

# -------------------------
# Component registration
# -------------------------
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "MonotSlamNode"
  EXECUTABLE monot_slam_node
)

# -------------------------
# Install
# -------------------------
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()