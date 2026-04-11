"""
Launch file for the Jetson.

Starts the base and gimbal stacks through their package launch files so they
can use their own default parameter files.
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import GroupAction, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    return LaunchDescription([
        GroupAction(
            actions=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        os.path.join(
                            get_package_share_directory('gimbal_control'),
                            'launch', 'gimbal_controller.launch.py')),
                ),
            ],
            forwarding=False
        ),
        GroupAction(
            actions=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        os.path.join(
                            get_package_share_directory('base_control'),
                            'launch', 'base_controller.launch.py')),
                ),
            ],
            forwarding=False
        ),
    ])
