"""Launch base controller node with an optional params file argument."""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_dir = get_package_share_directory('base_control')
    default_params_file = os.path.join(pkg_dir, 'config', 'base_controller.yaml')

    return LaunchDescription([
        DeclareLaunchArgument(
            'params_file',
            default_value=default_params_file,
            description='Path to the base controller parameters file'
        ),
        Node(
            package='base_control',
            executable='base_controller',
            name='base_controller',
            parameters=[LaunchConfiguration('params_file')],
            output='screen',
        ),
    ])
