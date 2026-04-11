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
// SPI (AS5048) pins
const int pin_spi_mosi = 23;
const int pin_spi_miso = 19;
const int pin_spi_clk  = 18;
const int pin_cs_yaw   = 2;
const int pin_cs_pitch = 0;

// Yaw motor
const int pin_yaw_in1 = 14;
const int pin_yaw_in2 = 27;
const int pin_yaw_in3 = 26;

// Pitch motor
const int pin_pitch_in1 = 25;
const int pin_pitch_in2 = 33;
const int pin_pitch_in3 = 32;

// ================================================================
// Motor params
// ================================================================
const int   motor_pole_pairs      = 11;
const float motor_phase_resistance = 5.50f / 2.0f;
const float motor_voltage_supply  = 12.0f;
const float motor_voltage_limit   = 6.0f;
const float motor_PID_velocity_P  = 0.7f;
const float motor_PID_velocity_I  = 10.0f;
const float motor_PID_velocity_D  = 0.0f;
const float motor_PID_velocity_output_ramp = 1000.0f;
const float motor_LPF_velocity_Tf = 0.01f;

// ================================================================
// micro-ROS
// ================================================================
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

rcl_timer_t curr_vel_timer;
const int curr_vel_pub_period = 100; // ms

// ================================================================
// MotorSide
// ================================================================
struct MotorSide
{
  // FOC
  MagneticSensorSPI sensor;
  BLDCMotor motor;
  BLDCDriver3PWM driver;

  // Control
  float targetVelocity;
  float lastAngle;
  bool  initialized;

  // micro-ROS
  const char* cmdTopic;
  const char* currTopic;

  rcl_subscription_t cmdSub;
  rcl_publisher_t    currPub;

  std_msgs__msg__Float32 cmdMsg;
  std_msgs__msg__Float32 currMsg;

  MotorSide(int in1, int in2, int in3, int csPin)
  : sensor(AS5048_SPI, csPin)
  , motor(motor_pole_pairs, motor_phase_resistance)
  , driver(in1, in2, in3)
  , targetVelocity(0.0f)
  , lastAngle(0.0f)
  , initialized(false)
  {}
};

// ================================================================
// Yaw / Pitch motors
// ================================================================
MotorSide motorYaw(
  pin_yaw_in1, pin_yaw_in2, pin_yaw_in3, pin_cs_yaw);

MotorSide motorPitch(
  pin_pitch_in1, pin_pitch_in2, pin_pitch_in3, pin_cs_pitch);

// Convenience array
MotorSide* motors[] = { &motorYaw, &motorPitch };
constexpr int motorCount = 2;

// ================================================================
// micro-ROS callbacks
// ================================================================
void handle_cmd_vel(MotorSide& m, const void* msgin)
{
  const auto* msg = static_cast<const std_msgs__msg__Float32*>(msgin);
  m.targetVelocity = msg->data;
}

void cmd_vel_yaw_cbk(const void* msgin)
{
  handle_cmd_vel(motorYaw, msgin);
}

void cmd_vel_pitch_cbk(const void* msgin)
{
  handle_cmd_vel(motorPitch, msgin);
}

void curr_vel_timer_cbk(rcl_timer_t* timer, int64_t)
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

// Safe to call before driver.init(): forces all PWM pins LOW at the GPIO level.
// motor.disable() / driver.disable() must NOT be called before driver.init()
// because driver->params is nullptr until then, causing a null-pointer crash
// inside _writeDutyCycle3PWM on ESP32.
void safeMotorPinsLow()
{
  const int pins[] = {
    pin_yaw_in1,   pin_yaw_in2,   pin_yaw_in3,
    pin_pitch_in1, pin_pitch_in2, pin_pitch_in3
  };
  for (int pin : pins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

void disableAllMotors()
{
  for (int i = 0; i < motorCount; ++i)
    motors[i]->motor.disable();
}

bool allMotorsInitialized()
{
  for (int i = 0; i < motorCount; ++i)
    if (!motors[i]->initialized) return false;
  return true;
}

void initMotor(MotorSide& m)
{
  m.sensor.init();
  m.motor.linkSensor(&m.sensor);

  m.driver.voltage_power_supply = motor_voltage_supply;
  m.driver.init();
  m.motor.linkDriver(&m.driver);

  m.motor.controller = MotionControlType::velocity;
  m.motor.PID_velocity.P = motor_PID_velocity_P;
  m.motor.PID_velocity.I = motor_PID_velocity_I;
  m.motor.PID_velocity.D = motor_PID_velocity_D;
  m.motor.PID_velocity.output_ramp = motor_PID_velocity_output_ramp;
  m.motor.voltage_limit = motor_voltage_limit;
  m.motor.LPF_velocity.Tf = motor_LPF_velocity_Tf;

  m.motor.init();
  m.initialized = m.motor.initFOC();
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
  // Force all motor PWM pins low immediately, before anything else.
  // This is to ensure output pins are in a known safe state even if
  // micro-ROS or motor init fails and prevent them from heating up due to DC lock.
  safeMotorPinsLow();

  // micro-ROS (may block here waiting for agent)
  set_microros_transports();

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "gimbal_hw_controller", "", &support);

  // Topics
  motorYaw.cmdTopic    = "/gimbal_yaw/cmd_vel";
  motorYaw.currTopic   = "/gimbal_yaw/curr_vel";
  motorPitch.cmdTopic  = "/gimbal_pitch/cmd_vel";
  motorPitch.currTopic = "/gimbal_pitch/curr_vel";

  // Executor: 2 subs + 1 timer
  rclc_executor_init(&executor, &support.context, motorCount + 1, &allocator);

  // Timer
  rclc_timer_init_default(
    &curr_vel_timer,
    &support,
    RCL_MS_TO_NS(curr_vel_pub_period),
    curr_vel_timer_cbk);

  rclc_executor_add_timer(&executor, &curr_vel_timer);

  // ROS entities per motor
  initMotorRos(motorYaw,   cmd_vel_yaw_cbk);
  initMotorRos(motorPitch, cmd_vel_pitch_cbk);

  // Motors
  for (int i = 0; i < motorCount; ++i) {
    initMotor(*motors[i]);
  }

  // Only start tasks if all motors initialized successfully.
  if (!allMotorsInitialized()) {
    disableAllMotors();
    return; // halt here; loop() will idle safely
    // TODO: instead of returning here, send some ros message to notify about the failure
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
