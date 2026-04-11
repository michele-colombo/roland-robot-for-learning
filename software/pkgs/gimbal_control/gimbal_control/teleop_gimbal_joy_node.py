"""
Teleop Gimbal Joy Node

Reads sensor_msgs/Joy and publishes geometry_msgs/Twist on /gimbal/cmd_vel.
Uses the right thumb stick (axes 3 & 4 on a standard Xbox controller).

FLU convention:
    angular.z  ->  yaw   (right stick horizontal, axis 3)
    angular.y  ->  pitch (right stick vertical,   axis 4)

Parameters:
    max_speed        (double) - rad/s at full stick deflection
    axis_yaw         (int)    - joystick axis index for yaw
    axis_pitch       (int)    - joystick axis index for pitch
    invert_yaw       (bool)   - flip yaw direction
    invert_pitch     (bool)   - flip pitch direction
"""

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from geometry_msgs.msg import Twist


class TeleopGimbalJoy(Node):

    def __init__(self):
        super().__init__('teleop_gimbal_joy')

        self.declare_parameter('max_speed', 5.0)
        self.declare_parameter('axis_yaw', 3)
        self.declare_parameter('axis_pitch', 4)
        self.declare_parameter('invert_yaw', False)
        self.declare_parameter('invert_pitch', False)

        self._read_parameters()
        self.add_on_set_parameters_callback(self._on_param_change)

        self.pub = self.create_publisher(Twist, '/gimbal/cmd_vel', 10)
        self.sub = self.create_subscription(Joy, '/joy', self._joy_callback, 10)

        self.get_logger().info(
            f'Teleop gimbal joy started  '
            f'[max_speed={self.max_speed:.2f} rad/s, '
            f'axes=({self.axis_yaw}, {self.axis_pitch})]'
        )

    def _read_parameters(self):
        self.max_speed = self.get_parameter('max_speed').value
        self.axis_yaw = self.get_parameter('axis_yaw').value
        self.axis_pitch = self.get_parameter('axis_pitch').value
        self.invert_yaw = self.get_parameter('invert_yaw').value
        self.invert_pitch = self.get_parameter('invert_pitch').value

    def _on_param_change(self, params):
        from rcl_interfaces.msg import SetParametersResult
        for p in params:
            if p.name == 'max_speed':
                self.max_speed = p.value
            elif p.name == 'axis_yaw':
                self.axis_yaw = p.value
            elif p.name == 'axis_pitch':
                self.axis_pitch = p.value
            elif p.name == 'invert_yaw':
                self.invert_yaw = p.value
            elif p.name == 'invert_pitch':
                self.invert_pitch = p.value
        self.get_logger().info('Parameters updated')
        return SetParametersResult(successful=True)

    def _joy_callback(self, msg: Joy):
        twist = Twist()

        if self.axis_yaw < len(msg.axes):
            yaw = msg.axes[self.axis_yaw] * self.max_speed
            twist.angular.z = -yaw if self.invert_yaw else yaw

        if self.axis_pitch < len(msg.axes):
            pitch = msg.axes[self.axis_pitch] * self.max_speed
            twist.angular.y = -pitch if self.invert_pitch else pitch

        self.pub.publish(twist)


def main(args=None):
    rclpy.init(args=args)
    node = TeleopGimbalJoy()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
