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
        get_package_share_directory('monot_slam'),
        'param',
        'config.yaml'
    )

    slam_node = Node(
        package='monot_slam',
        executable='monot_slam_node',
        name='monot_slam_node',
        parameters=[parameters],
        output='screen'
    )

    ld.add_action(slam_node)
    return ld