"""Launch base_controller with parameters from config/params.yaml."""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    pkg_dir = get_package_share_directory('base_control')
    params_file = os.path.join(pkg_dir, 'config', 'params.yaml')

    return LaunchDescription([
        Node(
            package='base_control',
            executable='base_controller',
            name='base_controller',
            parameters=[params_file],
            output='screen',
        ),
    ])
