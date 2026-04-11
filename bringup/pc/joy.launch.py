"""
Launch file for remote control on PC: joystick, base teleop, and gimbal teleop.

Starts:
    - joy_node            (reads the gamepad)
    - teleop_twist_joy    (left stick  -> /base/cmd_vel)
    - teleop_gimbal_joy   (right stick -> /gimbal/cmd_vel)
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    launch_dir = os.path.dirname(os.path.abspath(__file__))
    joy_config = os.path.join(launch_dir, 'xbox.config.yaml')

    gimbal_pkg_dir = get_package_share_directory('gimbal_control')
    gimbal_launch = os.path.join(gimbal_pkg_dir, 'launch', 'teleop_gimbal_joy.launch.py')

    return LaunchDescription([
        # ── Joystick driver ─────────────────────────────────────────
        Node(
            package='joy',
            executable='joy_node',
            name='joy_node',
            parameters=[joy_config],
            output='screen',
        ),

        # ── Base teleop (left stick -> /base/cmd_vel) ───────────────────
        Node(
            package='teleop_twist_joy',
            executable='teleop_node',
            name='teleop_twist_joy_node',
            parameters=[joy_config],
            output='screen',
            remappings=[('/cmd_vel', '/base/cmd_vel')],  # default is /cmd_vel, but our base controller listens on /base/cmd_vel
        ),

        # ── Gimbal teleop (right stick -> /gimbal/cmd_vel) ─────────
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(gimbal_launch),
        ),
    ])
