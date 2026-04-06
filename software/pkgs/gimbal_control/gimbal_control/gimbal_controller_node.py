"""
Gimbal Controller Node

Forwards angular velocity commands from /gimbal/cmd_vel (geometry_msgs/Twist)
to the individual gimbal motor topics as std_msgs/Float32.

FLU convention:
    angular.z  ->  /gimbal_yaw/cmd_velocity
    angular.y  ->  /gimbal_pitch/cmd_velocity

Parameters:
    invert_yaw       (bool)   - flip yaw output sign
    invert_pitch     (bool)   - flip pitch output sign
    max_velocity     (double) - rad/s, symmetric clamp
"""

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Float32


class GimbalController(Node):

    def __init__(self):
        super().__init__('gimbal_controller')

        self.declare_parameter('invert_yaw', False)
        self.declare_parameter('invert_pitch', False)
        self.declare_parameter('max_velocity', 50.0)

        self._read_parameters()
        self.add_on_set_parameters_callback(self._on_param_change)

        self.pub_yaw = self.create_publisher(
            Float32, '/gimbal_yaw/cmd_velocity', 10)
        self.pub_pitch = self.create_publisher(
            Float32, '/gimbal_pitch/cmd_velocity', 10)

        self.sub = self.create_subscription(
            Twist, '/gimbal/cmd_vel', self._cmd_vel_callback, 10)

        self.get_logger().info(
            f'Gimbal controller started  '
            f'[max_vel={self.max_velocity:.2f} rad/s]'
        )

    def _read_parameters(self):
        self.invert_yaw = self.get_parameter('invert_yaw').value
        self.invert_pitch = self.get_parameter('invert_pitch').value
        self.max_velocity = self.get_parameter('max_velocity').value

    def _on_param_change(self, params):
        from rcl_interfaces.msg import SetParametersResult
        for p in params:
            if p.name == 'invert_yaw':
                self.invert_yaw = p.value
            elif p.name == 'invert_pitch':
                self.invert_pitch = p.value
            elif p.name == 'max_velocity':
                self.max_velocity = p.value
        self.get_logger().info('Parameters updated')
        return SetParametersResult(successful=True)

    def _cmd_vel_callback(self, msg: Twist):
        v_yaw = msg.angular.z
        v_pitch = msg.angular.y

        if self.invert_yaw:
            v_yaw = -v_yaw
        if self.invert_pitch:
            v_pitch = -v_pitch

        # Clamp
        v_yaw = max(-self.max_velocity, min(self.max_velocity, v_yaw))
        v_pitch = max(-self.max_velocity, min(self.max_velocity, v_pitch))

        self.pub_yaw.publish(Float32(data=float(v_yaw)))
        self.pub_pitch.publish(Float32(data=float(v_pitch)))


def main(args=None):
    rclpy.init(args=args)
    node = GimbalController()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
