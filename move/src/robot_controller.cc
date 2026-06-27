#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/twist.hpp>

class RoutePublisher : public rclcpp::Node {

	public:
	RoutePublisher() : Node("route_pub") {
		cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
		// robot_pos = this->create_subscriber(); // @TODO fill in
	}

	void publish_cmd_vel()
	{
		// RCLCPP_INFO(this->get_logger(),"publishing cmd_vel");
		auto go_forward = geometry_msgs::msg::Twist();
		go_forward.linear.x = 1.0;
		cmd_vel_pub_->publish(go_forward);
	}
	private:
	rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;

};

int main(int argc, char** argv)
{
	rclcpp::init(argc, argv);
	auto node = std::make_shared<RoutePublisher>();
	bool status = true;
	while (status) {
		// RCLCPP_INFO(node->get_logger(),"entering cmd vel...");
		node->publish_cmd_vel();
		rclcpp::spin_some(node);
	};
	rclcpp::shutdown();
	return 0;
}
