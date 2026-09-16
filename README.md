# Lunabotics Application Follow Up: ROS2 Case Study

only have a limited number of spots, we want more information to guide second round of seleciton. also, get up to speed on ROS2. so, two purposes: 1) we can make a well informed decision about on the team and 2) once you compelte, you'll be good to hop on a specific subteam. 

Check github for updates! 

ROS2 Intro Slides: https://docs.google.com/presentation/d/15GYIs2856JcJFc202Pkjt8MY6tZNBv61R5_f19z1MYk/edit?usp=sharing

# Submission Guidelines  
- Download a zip of this repo and implemeent fixes locally
- email completed project to: jgerdsen@andrew.cmu.edu, angelaab@andrew.cmu.edu **by 9/23**
- if you don't finish, that's fine! we would suggest going through all tasks and writing out in natural language how you intended to solve. That way, if you're in a time crunch, you can submit what you have.
- AI Usage: You can use AI. However, code should remain human readable. Look for ways to demonstrate you can solve problems computationally and act as a systems thinker. Be able to answer questions about design decisions. 
---
# Project Writeup 

The `move` package is a **Gazebo sim testbed**: a differential-drive robot with a lidar
and an IMU, simulated in Gazebo and bridged into ROS 2 so you can see its sensor data and
its transforms in RViz.

## 0.: Gazebo Sim Testbed Bringup

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

By publishing data to a ceretain topic, the robot can move forward.

1) Identify what the topic is. note, as a sanity check if you run this command, the robot should move forwar

```bash
ros2 topic pub /topic_name geometry_msgs/msg/Twist "{linear: {x: 1.0}}" -r 10
```

2) write a publisher to the topic in the node (@TODO fill in pseudocode for this pub, e.g. self.move_pub = self._publisher("topic_name",QOS stuff...)) 

3) select a path to navigate the robot around the wall. In comments, Document why you choose this path and how you choose to represent it. 


## TASK 2: analyze error 
By receiving data from a certain topic, the robot can get information on its current position. Hint: This data comes from one of the two onboard sensors on the robot. 

1) Identify what the topic is. As a sanity check, this topic should have 6D data. 

2) write a subscriber to the topic in the node (@TODO fill in pseudocode for this subscriber, e.g. self.robot_pos_sub = self._subscriber("topic_name",QOS stuff...)) 

3) define a callback to the topic that compares the data received on the robots position to the actual information. it should also use the self.publish("/error") topic if it the delta is above self.error_thresh 

## TASK 3 (Stretch): Detect Obstructions 
Warning: this is more openeded! 

1) write a subscriber to receive lidar data
2) within the lidar data, we do not care about going through the barrier - think of this as sensor noise like dust in the air that won't really obstruct our robots progress. However, we do care about avoid the poles. 
3) In the subscription, come up with a way to identify obstacles. by implementing the function is_obstalce(point)
the code will then filter out all obstacles and publish a refined cloud to /obstacle_cloud

