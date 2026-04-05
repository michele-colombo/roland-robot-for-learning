"""
Base Controller Node

Converts geometry_msgs/Twist (FLU convention) into per-wheel velocity
commands for a differential-drive robot.

Differential drive kinematics (FLU):
    v_right = (v_linear + omega * wheel_distance / 2) / wheel_radius
    v_left  = (v_linear - omega * wheel_distance / 2) / wheel_radius

Subscriptions:
    /cmd_vel  (geometry_msgs/Twist)

Publications:
    /motor_left/cmd_velocity   (std_msgs/Float32)  - left wheel rad/s
    /motor_right/cmd_velocity  (std_msgs/Float32)  - right wheel rad/s
    /motor_left/velocity_clamped  (std_msgs/Bool)  - True while output is clamped
    /motor_right/velocity_clamped (std_msgs/Bool)  - True while output is clamped

Parameters (see config/params.yaml):
    wheel_radius         (double)  - metres
    wheel_distance       (double)  - metres, track width
    invert_left_wheel    (bool)    - flip left output sign
    invert_right_wheel   (bool)    - flip right output sign
    max_wheel_velocity   (double)  - rad/s, symmetric clamp
"""

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Float32, Bool


class BaseController(Node):

    def __init__(self):
        super().__init__('base_controller')

        # ── declare parameters ──────────────────────────────────────────
        self.declare_parameter('wheel_radius', 0.05)
        self.declare_parameter('wheel_distance', 0.3)
        self.declare_parameter('invert_left_wheel', False)
        self.declare_parameter('invert_right_wheel', False)
        self.declare_parameter('max_wheel_velocity', 50.0)

        # ── cache parameters ────────────────────────────────────────────
        self._read_parameters()

        # React to runtime parameter changes (e.g. via `ros2 param set`)
        self.add_on_set_parameters_callback(self._on_param_change)

        # ── publishers ──────────────────────────────────────────────────
        self.pub_left = self.create_publisher(Float32, '/motor_left/cmd_velocity', 10)
        self.pub_right = self.create_publisher(Float32, '/motor_right/cmd_velocity', 10)
        self.pub_left_clamped = self.create_publisher(Bool, '/motor_left/velocity_clamped', 10)
        self.pub_right_clamped = self.create_publisher(Bool, '/motor_right/velocity_clamped', 10)

        # ── subscriber ──────────────────────────────────────────────────
        self.sub_cmd_vel = self.create_subscription(
            Twist, '/cmd_vel', self._cmd_vel_callback, 10
        )

        self.get_logger().info(
            f'Base controller started  '
            f'[R={self.wheel_radius:.4f} m, '
            f'D={self.wheel_distance:.4f} m, '
            f'max_vel={self.max_wheel_velocity:.2f} rad/s]'
        )

    # ── parameter helpers ───────────────────────────────────────────────
    def _read_parameters(self):
        self.wheel_radius = self.get_parameter('wheel_radius').value
        self.wheel_distance = self.get_parameter('wheel_distance').value
        self.invert_left = self.get_parameter('invert_left_wheel').value
        self.invert_right = self.get_parameter('invert_right_wheel').value
        self.max_wheel_velocity = self.get_parameter('max_wheel_velocity').value

    def _on_param_change(self, params):
        """Called automatically when any parameter is changed at runtime."""
        from rcl_interfaces.msg import SetParametersResult
        for p in params:
            if p.name == 'wheel_radius':
                self.wheel_radius = p.value
            elif p.name == 'wheel_distance':
                self.wheel_distance = p.value
            elif p.name == 'invert_left_wheel':
                self.invert_left = p.value
            elif p.name == 'invert_right_wheel':
                self.invert_right = p.value
            elif p.name == 'max_wheel_velocity':
                self.max_wheel_velocity = p.value
        self.get_logger().info('Parameters updated')
        return SetParametersResult(successful=True)

    # ── main callback ───────────────────────────────────────────────────
    def _cmd_vel_callback(self, msg: Twist):
        v = msg.linear.x          # forward velocity   (m/s)
        omega = msg.angular.z     # yaw rate            (rad/s)

        # Differential drive inverse kinematics
        #   In FLU, positive omega turns LEFT (counter-clockwise from above).
        #   Left wheel is slower when turning left → subtract the term.
        v_left = (v - omega * self.wheel_distance / 2.0) / self.wheel_radius
        v_right = (v + omega * self.wheel_distance / 2.0) / self.wheel_radius

        # Optional inversion (e.g. when a motor is mounted mirrored)
        if self.invert_left:
            v_left = -v_left
        if self.invert_right:
            v_right = -v_right

        # Clamp to max wheel velocity
        left_clamped = abs(v_left) > self.max_wheel_velocity
        right_clamped = abs(v_right) > self.max_wheel_velocity

        if left_clamped:
            v_left = self.max_wheel_velocity if v_left > 0 else -self.max_wheel_velocity
        if right_clamped:
            v_right = self.max_wheel_velocity if v_right > 0 else -self.max_wheel_velocity

        # Publish wheel velocities
        self.pub_left.publish(Float32(data=float(v_left)))
        self.pub_right.publish(Float32(data=float(v_right)))

        # Publish clamp flags
        self.pub_left_clamped.publish(Bool(data=left_clamped))
        self.pub_right_clamped.publish(Bool(data=right_clamped))


def main(args=None):
    rclpy.init(args=args)
    node = BaseController()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
