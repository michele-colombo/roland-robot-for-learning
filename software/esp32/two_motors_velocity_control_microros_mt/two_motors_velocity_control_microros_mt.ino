#include <micro_ros_arduino.h>
#include <SimpleFOC.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/float32.h>

// ================================================================
// Pins
// ================================================================
const int pin_right_in1 = 32;
const int pin_right_in2 = 33;
const int pin_right_in3 = 25;

const int pin_left_in1  = 26;
const int pin_left_in2  = 27;
const int pin_left_in3  = 14;

// default i2c bus
const int pin_right_sda = 21;
const int pin_right_scl = 22;

// AS5600 has fixed address -> second I2C bus
const int pin_left_sda  = 19;
const int pin_left_scl  = 23;

// ================================================================
// Motor params
// ================================================================
const int   motor_pole_pairs      = 7;
const float motor_voltage_supply  = 12.0f;
const float motor_voltage_limit   = 6.0f;
const float motor_PID_velocity_P  = 0.05f;
const float motor_PID_velocity_I  = 1.0f;
const float motor_PID_velocity_D  = 0.0f;
const float motor_LPF_velocity_Tf = 0.01f;

// ================================================================
// micro-ROS
// ================================================================
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

rcl_timer_t curr_velocity_timer;
const int curr_velocity_pub_period = 100; // ms

// ================================================================
// MotorSide
// ================================================================
struct MotorSide
{
  // Hardware
  TwoWire* wire;

  // FOC
  MagneticSensorI2C sensor;
  BLDCMotor motor;
  BLDCDriver3PWM driver;

  // Control
  float targetVelocity;
  float lastAngle;

  // micro-ROS
  const char* cmdTopic;
  const char* currTopic;

  rcl_subscription_t cmdSub;
  rcl_publisher_t    currPub;

  std_msgs__msg__Float32 cmdMsg;
  std_msgs__msg__Float32 currMsg;

  MotorSide(int in1, int in2, int in3, TwoWire* w)
  : wire(w)
  , sensor(AS5600_I2C)
  , motor(motor_pole_pairs)
  , driver(in1, in2, in3)
  , targetVelocity(0.0f)
  , lastAngle(0.0f)
  {}
};

// ================================================================
// Left / Right motors
// ================================================================
MotorSide motorRight(
  pin_right_in1, pin_right_in2, pin_right_in3, &Wire);

MotorSide motorLeft(
  pin_left_in1, pin_left_in2, pin_left_in3, &Wire1);

// Convenience array
MotorSide* motors[] = { &motorRight, &motorLeft };
constexpr int motorCount = 2;

// ================================================================
// micro-ROS callbacks
// ================================================================
void handle_cmd_velocity(MotorSide& m, const void* msgin)
{
  const auto* msg = static_cast<const std_msgs__msg__Float32*>(msgin);
  m.targetVelocity = msg->data;
}

void cmd_velocity_right_cbk(const void* msgin)
{
  handle_cmd_velocity(motorRight, msgin);
}

void cmd_velocity_left_cbk(const void* msgin)
{
  handle_cmd_velocity(motorLeft, msgin);
}

void curr_velocity_timer_cbk(rcl_timer_t* timer, int64_t)
{
  if (!timer) return;

  static int64_t lastTime = 0;
  int64_t now = esp_timer_get_time();
  float dt = (now - lastTime) / 1e6f;
  lastTime = now;

  if (dt <= 0.0f) return;

  for (int i = 0; i < motorCount; ++i) {
    MotorSide& m = *motors[i];

    float angle = m.sensor.getAngle();
    float velocity = (angle - m.lastAngle) / dt;
    m.lastAngle = angle;

    m.currMsg.data = velocity;
    rcl_publish(&m.currPub, &m.currMsg, NULL);
  }
}

// ================================================================
// Helpers
// ================================================================
void initMotor(MotorSide& m)
{
  m.sensor.init(m.wire);
  m.motor.linkSensor(&m.sensor);

  m.driver.voltage_power_supply = motor_voltage_supply;
  m.driver.init();
  m.motor.linkDriver(&m.driver);

  m.motor.controller = MotionControlType::velocity;
  m.motor.PID_velocity.P = motor_PID_velocity_P;
  m.motor.PID_velocity.I = motor_PID_velocity_I;
  m.motor.PID_velocity.D = motor_PID_velocity_D;
  m.motor.voltage_limit = motor_voltage_limit;
  m.motor.LPF_velocity.Tf = motor_LPF_velocity_Tf;

  m.motor.init();
  m.motor.initFOC();
}

void initMotorRos(MotorSide& m, rclc_subscription_callback_t cbk)
{
  rclc_publisher_init_best_effort(
    &m.currPub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    m.currTopic);

  rclc_subscription_init_best_effort(
    &m.cmdSub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    m.cmdTopic);

  rclc_executor_add_subscription(
    &executor,
    &m.cmdSub,
    &m.cmdMsg,
    cbk,
    ON_NEW_DATA);
}


// ================================================================
// Tasks
// ================================================================
void ros_loop(void*)
{
  while (1) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void control_loop(void*)
{
  while (1) {
    for (int i = 0; i < motorCount; ++i) {
      MotorSide& m = *motors[i];
      m.motor.loopFOC();
      m.motor.move(m.targetVelocity);
    }
  }
}

// ================================================================
// Setup
// ================================================================
void setup()
{
  set_microros_transports();

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "base_controller", "", &support);

  // Topics
  motorRight.cmdTopic  = "/motor_right/cmd_velocity";
  motorRight.currTopic = "/motor_right/curr_velocity";
  motorLeft.cmdTopic   = "/motor_left/cmd_velocity";
  motorLeft.currTopic  = "/motor_left/curr_velocity";

  // Executor: 2 subs + 1 timer
  rclc_executor_init(&executor, &support.context, motorCount + 1, &allocator);

  // Timer
  rclc_timer_init_default(
    &curr_velocity_timer,
    &support,
    RCL_MS_TO_NS(curr_velocity_pub_period),
    curr_velocity_timer_cbk);

  rclc_executor_add_timer(&executor, &curr_velocity_timer);

  // ROS entities per motor
  initMotorRos(motorRight, cmd_velocity_right_cbk);
  initMotorRos(motorLeft,  cmd_velocity_left_cbk);

  // I2C
  Wire.setClock(400000);
  Wire1.setClock(400000);

  Wire.begin(pin_right_sda, pin_right_scl, 400000);
  Wire1.begin(pin_left_sda, pin_left_scl, 400000);

  // Motors
  for (int i = 0; i < motorCount; ++i) {
    initMotor(*motors[i]);
  }

  // Tasks
  xTaskCreatePinnedToCore(
    ros_loop, "ros_loop", 10000, NULL, 5, NULL, 0);

  xTaskCreatePinnedToCore(
    control_loop, "control_loop", 10000, NULL, 10, NULL, 1);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
