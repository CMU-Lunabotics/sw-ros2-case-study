// Minimal closed-loop controller for the sim testbed.
//
// Subscribes:  /lidar (sensor_msgs/LaserScan), /imu (sensor_msgs/Imu)
// Publishes:   /cmd_vel (geometry_msgs/Twist)
//
// Drives the vehicle forward and stops when the lidar sees something closer
// than kStopDistance. Deliberately simple -- this is the hook for onboarding
// exercises, not a real planner.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>

#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

using namespace std::chrono_literals;

class RoutePublisher : public rclcpp::Node {
 public:
  RoutePublisher() : Node("route_pub") {
    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    lidar_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "/lidar", rclcpp::SensorDataQoS(),
        std::bind(&RoutePublisher::on_scan, this, std::placeholders::_1));

    imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
        "/imu", rclcpp::SensorDataQoS(),
        std::bind(&RoutePublisher::on_imu, this, std::placeholders::_1));

    // Publish on a timer instead of a busy loop so the node yields to the executor.
    timer_ = this->create_wall_timer(100ms, std::bind(&RoutePublisher::publish_cmd_vel, this));

    RCLCPP_INFO(this->get_logger(), "route_pub up: /lidar + /imu -> /cmd_vel");
  }

 private:
  static constexpr double kStopDistance = 1.5;  // metres
  static constexpr double kForwardSpeed = 1.0;  // m/s

  void on_scan(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    double nearest = std::numeric_limits<double>::infinity();
    for (const float range : msg->ranges) {
      // Gazebo reports out-of-range returns as inf; ignore those and anything
      // below range_min, which is noise rather than an obstacle.
      if (std::isfinite(range) && range >= msg->range_min) {
        nearest = std::min(nearest, static_cast<double>(range));
      }
    }
    nearest_obstacle_ = nearest;
  }

  void on_imu(const sensor_msgs::msg::Imu::SharedPtr msg) {
    yaw_rate_ = msg->angular_velocity.z;
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                         "imu yaw rate %.3f rad/s, nearest obstacle %.2f m",
                         yaw_rate_, nearest_obstacle_);
  }

  void publish_cmd_vel() {
    auto cmd = geometry_msgs::msg::Twist();
    const bool blocked = nearest_obstacle_ < kStopDistance;
    cmd.linear.x = blocked ? 0.0 : kForwardSpeed;
    if (blocked) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                           "obstacle at %.2f m, holding", nearest_obstacle_);
    }
    cmd_vel_pub_->publish(cmd);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr lidar_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::TimerBase::SharedPtr timer_;

  double nearest_obstacle_ = std::numeric_limits<double>::infinity();
  double yaw_rate_ = 0.0;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RoutePublisher>());
  rclcpp::shutdown();
  return 0;
}
