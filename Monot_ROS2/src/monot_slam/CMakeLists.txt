cmake_minimum_required(VERSION 3.8)
project(monot_slam)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic
    -Wno-deprecated -Wno-reorder -Wno-unused-parameter
    -Wno-deprecated-declarations -Wno-sign-compare -Wno-pedantic
    -Wno-overloaded-virtual
    )
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(Pangolin REQUIRED)
find_package(Eigen3 REQUIRED)


# adds external library header.
include_directories(
    /home/racc_man/ORB-SLAM3/ORB_SLAM3/include
    /home/racc_man/ORB-SLAM3/ORB_SLAM3/include/CameraModels
    /home/racc_man/ORB-SLAM3/ORB_SLAM3/Thirdparty/Sophus
    /home/racc_man/ORB-SLAM3/ORB_SLAM3
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
)

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/monot_slam_node.cpp
)

target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  ${EIGEN3_LIBS}
  ${Pangolin_LIBRARIES}
  /home/racc_man/ORB-SLAM3/ORB_SLAM3/lib/libORB_SLAM3.so
  /home/racc_man/ORB-SLAM3/ORB_SLAM3/Thirdparty/DBoW2/lib/libDBoW2.so
  /home/racc_man/ORB-SLAM3/ORB_SLAM3/Thirdparty/g2o/lib/libg2o.so
  -lboost_serialization
  -lcrypto
)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH
    "/home/racc_man/ORB-SLAM3/ORB_SLAM3/lib"
)

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "MonotSlamNode"
  EXECUTABLE monot_slam_node
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package(
  INSTALL_TO_SHARE
)

install(FILES DESTINATION share/${PROJECT_NAME})
