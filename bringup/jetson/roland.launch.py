"""
Launch file for the Jetson.

Starts the base and gimbal stacks through their package launch files so they
can use their own default parameter files.
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    gimbal_pkg_dir = get_package_share_directory('gimbal_control')
    base_pkg_dir = get_package_share_directory('base_control')

    gimbal_launch = os.path.join(gimbal_pkg_dir, 'launch', 'gimbal_controller.launch.py')
    base_launch = os.path.join(base_pkg_dir, 'launch', 'base_controller.launch.py')

    return LaunchDescription([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(gimbal_launch),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(base_launch),
        ),
    ])
