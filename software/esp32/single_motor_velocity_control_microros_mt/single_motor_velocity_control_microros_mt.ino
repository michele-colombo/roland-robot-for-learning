#include <micro_ros_arduino.h>
#include <SimpleFOC.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/float32.h>

#define MOTOR_SIDE "right"

// ---------------- Pins ----------------
const int pinDriverIn1 = 25;
const int pinDriverIn2 = 33;
const int pinDriverIn3 = 32;
const int pinDriverEn  = 26;

const int pinSensorSda = 21;
const int pinSensorScl = 22;

// ---------------- Params ----------------
const int   motor_pole_pairs     = 7;
const float motor_voltage_supply = 12.0;
const float motor_voltage_limit  = 6.0;

// ---------------- micro-ROS ----------------
const char node_name[] = "motor_" MOTOR_SIDE "_node";
const char cmd_vel_topic_name[] = "/motor_" MOTOR_SIDE "/cmd_vel";
const char curr_vel_topic_name[] = "/motor_" MOTOR_SIDE "/curr_vel";
rcl_publisher_t curr_vel_pub;
rcl_subscription_t cmd_vel_sub;
rcl_timer_t curr_vel_timer;
int curr_vel_pub_interval = 100; // ms

std_msgs__msg__Float32 curr_vel_msg;
std_msgs__msg__Float32 cmd_vel_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

// ---------------- FOC ----------------
MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);
BLDCMotor motor = BLDCMotor(motor_pole_pairs);
BLDCDriver3PWM driver = BLDCDriver3PWM(
  pinDriverIn1, pinDriverIn2, pinDriverIn3, pinDriverEn);

// ---------------- Control ----------------
float target_velocity = 0.0f;

// ------------------------------------------------
// Subscriber callbacks
// ------------------------------------------------
void cmd_vel_sub_callback(const void * msgin)
{
  const std_msgs__msg__Float32 * msg =
    (const std_msgs__msg__Float32 *)msgin;

  target_velocity = msg->data;
}

// ------------------------------------------------
// Timer callbacks
// ------------------------------------------------
void curr_vel_timer_callback(
  rcl_timer_t * timer, int64_t last_call_time)
{
  (void) last_call_time;
  if (timer == NULL) {
    return;
  }

  static float last_angle = 0.0f;
  static int64_t last_time = 0;

  float curr_angle = sensor.getAngle();
  int64_t current_time = esp_timer_get_time();

  float velocity = -(curr_angle - last_angle) / ((current_time - last_time) / 1e6f);
  last_angle = curr_angle;
  last_time = current_time;

  curr_vel_msg.data = velocity;
  rcl_publish(&curr_vel_pub, &curr_vel_msg, NULL);

}

// ------------------------------------------------
// ROS executor loop
// ------------------------------------------------
void ros_loop(void *pvParameters)
{
  while (1) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// ------------------------------------------------
// Real-time FOC loop
// ------------------------------------------------
void control_loop(void *pvParameters)
{
  while (1) {
    motor.loopFOC();
    motor.move(target_velocity);
  }
}

// ------------------------------------------------
// Setup
// ------------------------------------------------
void setup()
{
  set_microros_transports();  

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, node_name, "", &support);

  // Publisher
  rclc_publisher_init_best_effort(
    &curr_vel_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    curr_vel_topic_name);

  // Subscriber
  rclc_subscription_init_best_effort(
    &cmd_vel_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    cmd_vel_topic_name);

  // Timer
  rclc_timer_init_default(
    &curr_vel_timer,
    &support,
    RCL_MS_TO_NS(curr_vel_pub_interval),
    curr_vel_timer_callback);

  // Executor (1 sub + 1 timer)
  rclc_executor_init(&executor, &support.context, 2, &allocator);

  rclc_executor_add_subscription(
    &executor,
    &cmd_vel_sub,
    &cmd_vel_msg,
    &cmd_vel_sub_callback,
    ON_NEW_DATA);

  rclc_executor_add_timer(
    &executor,
    &curr_vel_timer);

  // ----------- FOC setup -----------
  sensor.init();
  motor.linkSensor(&sensor);

  driver.voltage_power_supply = motor_voltage_supply;
  driver.init();
  motor.linkDriver(&driver);

  motor.controller = MotionControlType::velocity;
  motor.PID_velocity.P = 0.05f;
  motor.PID_velocity.I = 1;
  motor.PID_velocity.D = 0;
  motor.voltage_limit = motor_voltage_limit;
  motor.LPF_velocity.Tf = 0.01f;

  motor.init();
  motor.initFOC();

  // ----------- Tasks -----------
  xTaskCreatePinnedToCore(
    ros_loop,
    "ros_loop",
    10000,
    NULL,
    5,
    NULL,
    0);

  xTaskCreatePinnedToCore(
    control_loop,
    "control_loop",
    10000,
    NULL,
    10,
    NULL,
    1);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
