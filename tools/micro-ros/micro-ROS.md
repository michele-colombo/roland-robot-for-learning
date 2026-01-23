# micro-ROS setup

## Steps on PC

To change the ROS distribution to use for building/running dockers, change the value of `DOCKER_ROS_DISTRO` in `settings.sh`

### 1. Build docker

Run `docker.build.sh` to build a docker containing the micro-ROS agent compiled.

### 2. Run docker

Run `docker.run.sh` to run a container (with docker image previously built) interactively.

### 3. Run agent

You can then run `run_agent.sh` inside docker. It is mounted in the docker, together with all the repo content (path: `/exp-esp32`). Pass as arguments the same arguments you would pass to the agent node, for instance:
- `serial --dev /dev/ttyUSB0` for USB (serial) connection
- `udp4 --port 8888` for WiFi connection

You can also pass the script command to the `docker.run.sh` script, in order to run the docker with the agent in a single command. For instance: `./docker.run.sh ./run_agent.sh serial --dev /dev/ttyUSB0`

There is also a `run_agent_serial.sh` script that automatically finds the port the ESP32 is connected to via USB and starts the agent.

## Steps on ESP32

### 1. Setup Arduino IDE for ESP32

Check [this file](../../tools/esp32/setup%20Arduino%20IDE.md)

### 2. Flash micro-ros_publisher example

(Taken from [here](https://www.hackster.io/514301/micro-ros-on-esp32-using-arduino-ide-1360ca))

Download the precompiled micro-ROS library for arduino IDE from [here](https://github.com/micro-ROS/micro_ros_arduino/releases). Extract it and put it in `/home/$USERNAME/Arduino/libraries`.

### 3. Compile and upload some microROS code.

### 4. Reset ESP32 after agent started
Press the RESET button after the agent has been started.

## Troubleshoot
### No topic with correct agent connection
If the agent correctly displays the connection (create_publisher, create_datawriter, etc.), but you cannot see any topic from ROS, it is usually enough to start the agent again, reset the ESP32 and run ros again. Check that any other process has the serial open.

### Timers and subscribers limited to 1Hz
A temporary solution is to downgrade the ESP32 Board Manager on Arduino IDE to 2.0.17, as suggested [here](https://www.reddit.com/r/ROS/comments/1fbqk5i/microros_seems_to_have_a_publish_limit_of_1/). Unfortunately this causes a mismatch with SimpleFOC, which requires ESP-IDF 5.x in the latest version. Thus SimpleFOC needs to be downgraded to version 2.3.3, too.