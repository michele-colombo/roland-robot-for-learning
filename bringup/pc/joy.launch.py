import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
  launch_dir = os.path.dirname(os.path.abspath(__file__))
  config = os.path.join(launch_dir, 'xbox.config.yaml')

  # Nodes to read joystick and convert to Twist message
  joy_nodes = IncludeLaunchDescription(
    PythonLaunchDescriptionSource(
      os.path.join(get_package_share_directory('teleop_twist_joy'), 'launch', 'teleop-launch.py')
    ),
    launch_arguments={'config_filepath': config}.items()
  )

  return LaunchDescription([joy_nodes])
