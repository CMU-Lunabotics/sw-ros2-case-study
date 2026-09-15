# Starter Project

# submit by 9/23 

The `move` package is a **Gazebo sim testbed**: a differential-drive robot with a lidar
and an IMU, simulated in Gazebo and bridged into ROS 2 so you can see its sensor data and
its transforms in RViz.

---
## Setup: Gazebo Sim Testbed Bringup

Note, you need a machine running. Ubuntu with **ROS 2 Jazzy** and **Gazebo Harmonic** (`ros_gz_sim`, `ros_gz_bridge`)


### 1. Build the workspace

```bash
cd ~/Desktop/onboarding
source /opt/ros/jazzy/setup.bash
colcon build --packages-select move
source install/setup.bash
```

`source install/setup.bash` is required in **every** new terminal. Skipping it is the most
common cause of "package 'move' not found".

### 2. Bring up the sim

```bash
ros2 launch move sim.launch.py
```

You should get a Gazebo window with a blue robot on a ground plane, and an RViz window
showing the same robot with its lidar returns.

The robot **sits still** on startup. That is intentional — nothing is commanding it yet.


You can access the following data from simulation: lidar + imu 

```bash
ros2 topic echo /lidar --once     # LaserScan, 640 ranges, frame_id: chassis
ros2 topic echo /imu --once       # Imu, frame_id: chassis
ros2 topic hz /lidar              # ~10 Hz (lower if the sim is running slow)
```

---

## Layout

```
move/
  launch/sim.launch.py              bringup: gazebo + bridge + TF + RViz
  models/vehicle_blue/model.sdf     the robot (single source of truth)
  models/vehicle_blue/model.config  model metadata for model:// lookup
  worlds/building_robot.sdf         the world; <include>s the model above
  rviz/vehicle.rviz                 RViz displays + fixed frame
  src/robot_controller.cc           example node: sensors in, cmd_vel out
```

The world **includes** the model rather than duplicating it, so the robot is defined in
exactly one place. `robot_state_publisher` reads that same `model.sdf` for
`robot_description`.

---

## TASK 1: Make it move 

### 5. Drive the robot

```bash
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 1.0}}" -r 10
```

Or run the bundled controller, which subscribes to `/lidar` and `/imu` and publishes
`/cmd_vel`, stopping when the lidar sees an obstacle within 1.5 m:

```bash
ros2 run move mover
# or bring the whole stack up with it enabled:
ros2 launch move sim.launch.py controller:=true
```

The world contains a wall at x=6 and a pillar at (4, 2.5) so the lidar has something to
return. With the controller enabled the robot drives forward and stops ~1.5 m short of the
wall.

## TASK 2: Detect Obstructions 

