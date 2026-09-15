# Starter Project

Use this as a starting point for understanding core concepts, libraries, and workflows
underlying the autonomy stack.

The `move` package is a **Gazebo sim testbed**: a differential-drive robot with a lidar
and an IMU, simulated in Gazebo and bridged into ROS 2 so you can see its sensor data and
its transforms in RViz.

---

## Gazebo Sim Testbed Bringup

### Prerequisites

- Ubuntu with **ROS 2 Jazzy** and **Gazebo Harmonic** (`ros_gz_sim`, `ros_gz_bridge`)
- See `START_HERE.md` for host/VM setup.

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

Useful launch arguments:

| Argument | Default | Effect |
|---|---|---|
| `rviz` | `true` | Set `rviz:=false` to skip RViz (useful over SSH) |
| `controller` | `false` | Set `controller:=true` to run the forward-drive node |

### 3. Verify the bridge

In a second terminal (remember to source):

```bash
ros2 topic list
```

Expected: `/clock`, `/cmd_vel`, `/imu`, `/joint_states`, `/lidar`,
`/model/vehicle_blue/odometry`, `/robot_description`, `/tf`, `/tf_static`.

Check real data is flowing:

```bash
ros2 topic echo /lidar --once     # LaserScan, 640 ranges, frame_id: chassis
ros2 topic echo /imu --once       # Imu, frame_id: chassis
ros2 topic hz /lidar              # ~10 Hz (lower if the sim is running slow)
```

### 4. Verify the transform tree

```bash
ros2 run tf2_ros tf2_echo map left_wheel
```

The tree is `map -> odom -> chassis -> {left_wheel, right_wheel}`. `map -> odom` comes from
a static transform in the launch file, `odom -> chassis` from Gazebo's odometry, and the
wheel transforms from `robot_state_publisher` driven by `/joint_states`.

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

## Troubleshooting

**"Detected jump back in time" / "Moved backwards in time"**
Two sims are running at once, so there are two `/clock` publishers. Check with
`ros2 topic info /clock` — publisher count must be `1`. Kill strays:
```bash
pkill -f gz-sim; pkill -f parameter_bridge; pkill -f robot_state_publisher
```
This also happens if a node in a sim-time stack is missing `use_sim_time: True`.

**Nothing visible in RViz**
RViz defaults its fixed frame to `map`. The launch file publishes `map -> odom` and ships
`rviz/vehicle.rviz` with the fixed frame preset, so use the bundled config (the launch file
passes it with `-d`) rather than a blank RViz.

**Robot missing in Gazebo**
The world resolves the robot via `model://vehicle_blue`, which needs
`GZ_SIM_RESOURCE_PATH` to include `install/move/share/move/models`. The launch file sets
this; if you run `gz sim` by hand, export it yourself.

**`XML Element[gz_frame_id] ... not defined in SDF`**
Harmless. `gz_frame_id` is a gz-sim extension that is not in the SDF schema; it is what
makes the sensors report `frame_id: chassis` instead of a scoped name.

**No lidar points in RViz**
Check the scan is actually hitting something:
```bash
ros2 topic echo /lidar --once --full-length
```
If every entry under `ranges:` is `.inf`, the sensor is working but nothing is within its
10 m range — RViz draws nothing for infinite returns. Add geometry to
`move/worlds/building_robot.sdf`. Note plain `ros2 topic echo /lidar` truncates the 640
ranges with `'...'`; `--full-length` is needed to see them.

**`SDFormat link [chassis] has a <sensor>, but URDF does not support this`**
Harmless. URDF has no sensor concept, so `sdformat_urdf` drops the sensor tags when
building the TF tree. The sensors still run in Gazebo.
