"""Launch teleop_gimbal_joy node with an optional params file argument."""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_dir = get_package_share_directory('gimbal_control')
    default_params_file = os.path.join(pkg_dir, 'config', 'teleop_gimbal_joy.yaml')

    return LaunchDescription([
        DeclareLaunchArgument(
            'params_file',
            default_value=default_params_file,
            description='Path to the teleop gimbal joy params file',
        ),
        Node(
            package='gimbal_control',
            executable='teleop_gimbal_joy_node',
            name='teleop_gimbal_joy',
            parameters=[LaunchConfiguration('params_file')],
            output='screen',
        ),
    ])
