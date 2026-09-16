# Lunabotics Application Follow Up: ROS2 Case Study

We only have a limited number of spots, so we want more information to guide the second
round of selection. We would also like you to get up to speed on ROS 2. This exercise
therefore serves two purposes: first, it lets us make a well informed decision about the
team, and second, once you complete it you will be ready to hop onto a specific subteam.

Check GitHub for updates!

ROS2 Intro Slides: https://docs.google.com/presentation/d/15GYIs2856JcJFc202Pkjt8MY6tZNBv61R5_f19z1MYk/edit?usp=sharing

# Submission Guidelines

- Download a zip of this repo and implement your fixes locally.
- Email your completed project to jgerdsen@andrew.cmu.edu and angelaab@andrew.cmu.edu **by 9/23**.
- If you do not finish, that is fine. We suggest working through all of the tasks and writing out in natural language how you intended to solve them. That way, if you are in a time crunch, you can submit what you have.
- On AI usage: you can use AI. However, your code should remain human readable. Look for ways to demonstrate that you can solve problems computationally and act as a systems thinker, and be ready to answer questions about your design decisions.

---

# Project Writeup

The `move` package is a **Gazebo sim testbed**: a differential-drive robot with a lidar
and an IMU, simulated in Gazebo and bridged into ROS 2 so you can see its sensor data and
its transforms in RViz.

## 0.: Gazebo Sim Testbed Bringup

Note that you need a machine running Ubuntu with **ROS 2 Jazzy** and **Gazebo Harmonic**
(`ros_gz_sim`, `ros_gz_bridge`).

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

You can access the following data from the simulation, the lidar and the IMU:

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
  src/publisher.py                  starter node: fill in TASKS 1-3 here
```

The world **includes** the model rather than duplicating it, so the robot is defined in
exactly one place. `robot_state_publisher` reads that same `model.sdf` for
`robot_description`.

---

## TASK 1: Make it move

By publishing data to a certain topic, the robot can move forward.

1) Identify what the topic is. As a sanity check, if you run the following command, the robot should move forward.

```bash
ros2 topic pub /topic_name geometry_msgs/msg/Twist "{linear: {x: 1.0}}" -r 10
```

2) Write a publisher to that topic in the node. (@TODO fill in pseudocode for this publisher, e.g. self.move_pub = self._publisher("topic_name", QOS stuff...))

3) Select a path to navigate the robot around the wall. In your comments, document why you chose this path and how you chose to represent it.


## TASK 2: Analyze error

By receiving data from a certain topic, the robot can get information about its current position. As a hint, this data comes from one of the two onboard sensors on the robot.

1) Identify what the topic is. As a sanity check, this topic should carry 6D data.

2) Write a subscriber to that topic in the node. (@TODO fill in pseudocode for this subscriber, e.g. self.robot_pos_sub = self._subscriber("topic_name", QOS stuff...))

3) Define a callback for the topic that compares the data it receives about the robot's position against the actual information. The callback should also publish on the self.publish("/error") topic whenever the delta is above self.error_thresh.

## TASK 3 (Stretch): Detect Obstructions

Warning: this task is more open-ended!

1) Write a subscriber to receive the lidar data.

2) Within the lidar data, we do not care about going through the barrier. Think of it as sensor noise, like dust in the air that will not really obstruct the robot's progress. We do, however, care about avoiding the poles.

3) In the subscription, come up with a way to identify obstacles by implementing the function is_obstacle(point). The code will then filter out all obstacles and publish a refined cloud to /obstacle_cloud.
