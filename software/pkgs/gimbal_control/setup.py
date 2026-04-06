import os
from glob import glob
from setuptools import find_packages, setup

package_name = 'gimbal_control'

setup(
    name=package_name,
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'config'),
            glob('config/*.yaml')),
        (os.path.join('share', package_name, 'launch'),
            glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    description='Gimbal teleop and controller nodes',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'teleop_gimbal_joy = gimbal_control.teleop_gimbal_joy_node:main',
            'gimbal_controller = gimbal_control.gimbal_controller_node:main',
        ],
    },
)
