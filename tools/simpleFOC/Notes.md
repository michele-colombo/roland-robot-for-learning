# Notes
- If you lower the voltage limit (like `motor.voltage_limit = 4;`) you get a lower maximum speed and lower torwue, but also a much smoother and quiter movement (maybe also cooler motor, to check).
- Maybe the speed of the control loop impacts on the motor temperature: it seems that a faster control loop keeps the motor cooler. Probably is asking to the motor a speed higher than the one it can reach that increases temperature.
- You need to call `motor.loopFOC()` at least once before calling `motor.move(target)`, thus they must be in the correct order in the control loop.