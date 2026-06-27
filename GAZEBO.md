## Basics 

On a machine with Ubuntu 22.04 (Jammy Jellyfish) and ROS2 Humble installed, install the default gazebo pairing with 
`sudo apt-get install ros-${ROS_DISTRO}-ros-gz` 

### ROS-Gazebo communication 

1. open three terminals

T1 (ROS/Gazebo Bridge): 
`ros2 run ros_gz_bridge parameter_bridge /keyboard/keypress@std_msgs/msg/Int32@ignition.msgs.Int32`
T2 (Gazebo Publisher): 
`ign gazebo empty.sdf` 
Then, by clicking menu bar in upper right corner of gui, add a keyboard publisher. 

T3 (ROS Subscriber): 
`ros2 topic echo /keyboard/keypress`

Sanity Check: clicking on the gazebo gui and typing should lead to visible data packets in T3. 

You can replicate this by intializing the bridge using a yaml file 
`ros2 run ros_gz_bridge parameter_bridge --ros-args -p config_file:=path/to/config.yaml`

Similary, with xml and python launch files. (See detailed docs [here](https://github.com/gazebosim/ros_gz/blob/ros2/ros_gz_bridge/README.md))
