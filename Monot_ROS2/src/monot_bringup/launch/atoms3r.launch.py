import os

import launch
import launch.actions
import launch.events

import launch_ros
import launch_ros.actions
import launch_ros.events

from launch import LaunchDescription
from launch_ros.actions import LifecycleNode
from launch_ros.actions import Node


from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    ld = launch.LaunchDescription()

    parameters = os.path.join(
        get_package_share_directory('monot_bringup'),
        'param',
        'config.yaml'
    )

    udp_imu_reciever_node = Node(
        package='monot_bringup',
        executable='udp_imu_reciever',
        name='udp_imu_reciever',
        parameters=[parameters],
        output='screen'
    )
    ld.add_action(udp_imu_reciever_node)

    udp_image_reciever_node = Node(
        package='monot_bringup',
        executable='udp_image_reciever',
        name='udp_image_reciever',
        parameters=[parameters],
        output='screen'
    )
    ld.add_action(udp_image_reciever_node)
    
    return ld