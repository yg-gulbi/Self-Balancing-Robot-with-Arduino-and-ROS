#!/usr/bin/env python3
import math
import time

import rospy
from dynamic_reconfigure.server import Server
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Imu

from balance_robot_control.cfg import PIDConfig


class PIDController:
    def __init__(self):
        rospy.init_node('pid_controler', anonymous=True)

        self.Kp_angle = 6.0
        self.Kd_angle = -0.75
        self.Kp_speed = 0.3
        self.Kp_steering = 0.95
        self.Kd_steering = -0.008
        self.max_output = 500

        self.setpoint_angle = 0.0
        self.prev_pitch_angle = 0.0
        self.setpoint_speed = 0.0
        self.current_speed = 0.0
        self.setpoint_steering = 0.0
        self.prev_roll_angle = 0.0

        self.last_cmd_time = time.time()
        self.cmd_timeout = 1.0
        self.prev_time = time.time()

        self.imu_subscriber = rospy.Subscriber('/imu', Imu, self.imu_callback)
        self.before_vel_subscriber = rospy.Subscriber('/before_vel', Twist, self.before_vel_callback)
        self.odom_subscriber = rospy.Subscriber('/odom', Odometry, self.odom_callback)
        self.cmd_vel_publisher = rospy.Publisher('/cmd_vel', Twist, queue_size=10)

        self.last_log_time = rospy.get_time()
        self.log_interval = 1
        self.rate = rospy.Rate(200)
        self.srv = Server(PIDConfig, self.reconfigure_callback)

    def reconfigure_callback(self, config, level):
        self.Kp_angle = config["Kp_angle"]
        self.Kd_angle = config["Kd_angle"]
        self.Kp_speed = config["Kp_speed"]
        self.Kp_steering = config["Kp_steering"]
        self.Kd_steering = config["Kd_steering"]

        rospy.loginfo(f"Reconfigured PID: {config}")
        return config

    def check_reset(self, current_time):
        if current_time < self.prev_time:
            rospy.logwarn("Simulation reset detected. Reinitializing PID state.")
            self.prev_time = current_time
            self.prev_pitch_angle = 0.0
            self.prev_roll_angle = 0.0
            self.setpoint_speed = 0.0
            self.setpoint_steering = 0.0
            cmd = Twist()
            cmd.linear.x = 0.0
            cmd.angular.z = 0.0
            try:
                self.cmd_vel_publisher.publish(cmd)
            except rospy.ROSException:
                rospy.logwarn("cmd_vel topic closed during reset.")

    def imu_callback(self, msg):
        current_pitch_angle = self.get_pitch(msg)
        current_roll_angle = self.get_roll(msg)
        current_time = rospy.Time.now().to_sec()

        self.check_reset(current_time)

        dt = current_time - self.prev_time
        if dt <= 0:
            dt = 1e-3
        pitch_rate = (current_pitch_angle - self.prev_pitch_angle) / dt
        roll_rate = (current_roll_angle - self.prev_roll_angle) / dt
        self.prev_time = current_time

        if abs(current_pitch_angle) > math.radians(70):
            cmd = Twist()
            cmd.linear.x = 0.0
            cmd.angular.z = 0.0
            try:
                self.cmd_vel_publisher.publish(cmd)
            except rospy.ROSException:
                rospy.logwarn("cmd_vel topic closed.")
            return

        if abs(self.setpoint_angle) > math.radians(45):
            self.setpoint_angle = math.radians(45)

        speed_error = self.setpoint_speed - self.current_speed
        self.setpoint_angle = self.Kp_speed * speed_error
        angle_error = self.setpoint_angle - current_pitch_angle
        output_angle = self.Kp_angle * angle_error + self.Kd_angle * pitch_rate
        output_steering = -(self.Kp_steering * self.setpoint_steering + self.Kd_steering * roll_rate)

        cmd = Twist()
        cmd.linear.x = output_angle
        cmd.angular.z = output_steering
        try:
            self.cmd_vel_publisher.publish(cmd)
        except rospy.ROSException:
            rospy.logwarn("cmd_vel topic closed.")

        self.prev_pitch_angle = current_pitch_angle
        self.prev_roll_angle = current_roll_angle

        if current_time - self.last_log_time >= self.log_interval:
            rospy.loginfo(
                f"setpoint_angle: {self.setpoint_angle:.3f}, "
                f"current_angle: {current_pitch_angle:.3f}, "
                f"speed: {self.current_speed:.3f}"
            )
            self.last_log_time = current_time

        if current_time - self.last_cmd_time > self.cmd_timeout:
            self.setpoint_speed = 0.0
            self.setpoint_steering = 0.0

    def before_vel_callback(self, msg):
        self.setpoint_speed = msg.linear.x
        self.setpoint_steering = msg.angular.z
        self.last_cmd_time = time.time()

    def odom_callback(self, msg):
        self.current_speed = msg.twist.twist.linear.x

    def get_pitch(self, imu_msg):
        _, pitch, _ = self.quaternion_to_euler(
            imu_msg.orientation.x,
            imu_msg.orientation.y,
            imu_msg.orientation.z,
            imu_msg.orientation.w
        )
        return pitch

    def get_roll(self, imu_msg):
        roll, _, _ = self.quaternion_to_euler(
            imu_msg.orientation.x,
            imu_msg.orientation.y,
            imu_msg.orientation.z,
            imu_msg.orientation.w
        )
        return roll

    def quaternion_to_euler(self, x, y, z, w):
        t0 = +2.0 * (w * x + y * z)
        t1 = +1.0 - 2.0 * (x * x + y * y)
        roll = math.atan2(t0, t1)

        t2 = +2.0 * (w * y - z * x)
        t2 = +1.0 if t2 > +1.0 else t2
        t2 = -1.0 if t2 < -1.0 else t2
        pitch = math.asin(t2)

        t3 = +2.0 * (w * z + x * y)
        t4 = +1.0 - 2.0 * (y * y + z * z)
        yaw = math.atan2(t3, t4)
        return roll, pitch, yaw

    def run(self):
        while not rospy.is_shutdown():
            self.rate.sleep()


if __name__ == '__main__':
    controller = PIDController()
    controller.run()
