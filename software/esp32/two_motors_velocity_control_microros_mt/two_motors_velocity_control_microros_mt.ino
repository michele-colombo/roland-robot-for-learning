#include <micro_ros_arduino.h>
#include <SimpleFOC.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/float32.h>

// ---------------- Pins ----------------
const int pin_right_in1 = 32;
const int pin_right_in2 = 33;
const int pin_right_in3 = 25;

const int pin_left_in1 = 26;
const int pin_left_in2 = 27;
const int pin_left_in3 = 14;

// default i2c bus
const int pin_right_sda = 21;
const int pin_right_scl = 22;

// AS5600 have fixed i2c address, so we'll use the second i2c bus
const int pin_left_sda = 19;
const int pin_left_scl = 23;

// ---------------- Params ----------------
const int   motor_pole_pairs     = 7;
const float motor_voltage_supply = 12.0;
const float motor_voltage_limit  = 6.0;
const float motor_PID_velocity_P = 0.05f;
const float motor_PID_velocity_I = 1.0f;
const float motor_PID_velocity_D = 0.0f;
const float motor_LPF_velocity_Tf = 0.01f;

// ---------------- micro-ROS ----------------
rcl_subscription_t cmd_velocity_right_sub;
rcl_subscription_t cmd_velocity_left_sub;
rcl_publisher_t curr_velocity_right_pub;
rcl_publisher_t curr_velocity_left_pub;
rcl_timer_t curr_velocity_timer;
const int curr_velocity_pub_period = 100; // ms

std_msgs__msg__Float32 cmd_velocity_right_msg;
std_msgs__msg__Float32 cmd_velocity_left_msg;
std_msgs__msg__Float32 curr_velocity_right_msg;
std_msgs__msg__Float32 curr_velocity_left_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

// ---------------- FOC ----------------
MagneticSensorI2C sensor_right = MagneticSensorI2C(AS5600_I2C);
BLDCMotor motor_right = BLDCMotor(motor_pole_pairs);
BLDCDriver3PWM driver_right = BLDCDriver3PWM(
  pin_right_in1, pin_right_in2, pin_right_in3);

MagneticSensorI2C sensor_left = MagneticSensorI2C(AS5600_I2C);
BLDCMotor motor_left = BLDCMotor(motor_pole_pairs);
BLDCDriver3PWM driver_left = BLDCDriver3PWM(
  pin_left_in1, pin_left_in2, pin_left_in3);

// ---------------- Control ----------------
float target_velocity_right = 0.0f;
float target_velocity_left = 0.0f;

// ------------------------------------------------
// Subscriber cbks
// ------------------------------------------------
void cmd_velocity_right_sub_cbk(const void * msgin)
{
  const std_msgs__msg__Float32 * msg =
    (const std_msgs__msg__Float32 *)msgin;

  target_velocity_right = msg->data;
}

void cmd_velocity_left_sub_cbk(const void * msgin)
{
  const std_msgs__msg__Float32 * msg =
    (const std_msgs__msg__Float32 *)msgin;

  target_velocity_left = msg->data;
}

// ------------------------------------------------
// Timer cbks
// ------------------------------------------------
void curr_velocity_timer_cbk(
  rcl_timer_t * timer, int64_t last_call_time)
{
  (void) last_call_time;
  if (timer == NULL) {
    return;
  }

  static float last_angle_right  = 0.0f;
  static float last_angle_left   = 0.0f;
  static int64_t last_time = 0;

  float curr_angle_right = sensor_right.getAngle();
  float curr_angle_left = sensor_left.getAngle();
  int64_t current_time = esp_timer_get_time();

  float velocity_right = -(curr_angle_right - last_angle_right) / ((current_time - last_time) / 1e6f);
  float velocity_left = -(curr_angle_left - last_angle_left) / ((current_time - last_time) / 1e6f);
  last_angle_right = curr_angle_right;
  last_angle_left = curr_angle_left;
  last_time = current_time;

  curr_velocity_right_msg.data = velocity_right;
  rcl_publish(&curr_velocity_right_pub, &curr_velocity_right_msg, NULL);

  curr_velocity_left_msg.data = velocity_left;
  rcl_publish(&curr_velocity_left_pub, &curr_velocity_left_msg, NULL);
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
    motor_right.loopFOC();
    motor_right.move(target_velocity_right);
    motor_left.loopFOC();
    motor_left.move(target_velocity_left);
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
  rclc_node_init_default(&node, "base_controller", "", &support);

  // Publisher
  rclc_publisher_init_best_effort(
    &curr_velocity_right_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "/motor_right/curr_velocity");

  rclc_publisher_init_best_effort(
    &curr_velocity_left_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "/motor_left/curr_velocity");

  // Subscriber
  rclc_subscription_init_best_effort(
    &cmd_velocity_right_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "/motor_right/cmd_velocity");

  rclc_subscription_init_best_effort(
    &cmd_velocity_left_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "/motor_left/cmd_velocity");

  // Timer
  rclc_timer_init_default(
    &curr_velocity_timer,
    &support,
    RCL_MS_TO_NS(curr_velocity_pub_period),
    curr_velocity_timer_cbk);

  // Executor (2 sub + 1 timer)
  rclc_executor_init(&executor, &support.context, 3, &allocator);

  rclc_executor_add_subscription(
    &executor,
    &cmd_velocity_right_sub,
    &cmd_velocity_right_msg,
    &cmd_velocity_right_sub_cbk,
    ON_NEW_DATA);

  rclc_executor_add_subscription(
    &executor,
    &cmd_velocity_left_sub,
    &cmd_velocity_left_msg,
    &cmd_velocity_left_sub_cbk,
    ON_NEW_DATA);

  rclc_executor_add_timer(
    &executor,
    &curr_velocity_timer);

  // ----------- i2c setup -----------
  Wire.setClock(400000);
  Wire1.setClock(400000);

  Wire.begin(pin_left_sda, pin_left_scl, (uint32_t)400000);
  Wire1.begin(pin_right_sda, pin_right_scl, (uint32_t)400000);

  // ----------- FOC setup -----------
  sensor_right.init(&Wire1);
  motor_right.linkSensor(&sensor_right);

  sensor_left.init(&Wire);
  motor_left.linkSensor(&sensor_left);

  driver_right.voltage_power_supply = motor_voltage_supply;
  driver_right.init();
  motor_right.linkDriver(&driver_right);

  driver_left.voltage_power_supply = motor_voltage_supply;
  driver_left.init();
  motor_left.linkDriver(&driver_left);

  motor_right.controller = MotionControlType::velocity;
  motor_right.PID_velocity.P = motor_PID_velocity_P;
  motor_right.PID_velocity.I = motor_PID_velocity_I;
  motor_right.PID_velocity.D = motor_PID_velocity_D;
  motor_right.voltage_limit = motor_voltage_limit;
  motor_right.LPF_velocity.Tf = motor_LPF_velocity_Tf;
  motor_right.init();
  motor_right.initFOC();

  motor_left.controller = MotionControlType::velocity;
  motor_left.PID_velocity.P = motor_PID_velocity_P;
  motor_left.PID_velocity.I = motor_PID_velocity_I;
  motor_left.PID_velocity.D = motor_PID_velocity_D;
  motor_left.voltage_limit = motor_voltage_limit;
  motor_left.LPF_velocity.Tf = motor_LPF_velocity_Tf;
  motor_left.init();
  motor_left.initFOC();

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
