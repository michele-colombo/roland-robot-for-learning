# ROLAND: a RObot for Learning Autonomous Navigation and Detection
ROLAND is a modular robotics project developed as a learning platform for autonomous navigation, SLAM, sensor fusion, and 3D reconstruction.
The system is built around a differential-drive mobile base and is designed to evolve over time as new sensing, perception, and actuation modules are added.

High-level autonomy runs on an NVIDIA Jetson Orin Nano, while low-level control is handled by an ESP32 driving brushless motors with FOC control.
The robot features self-designed, 3D-printed parts.

## Roadmap
- [x] Differential drive base
- [ ] Add 2D LiDAR and off-the-shelf SLAM module
- [ ] Add stereo camera and 3D mapping of the scene
- [ ] Implement object seeking from image reference
- [ ] Add a manipulator for grabbing the object?
