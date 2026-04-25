#!/usr/bin/env python3
import rospy
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Imu
from nav_msgs.msg import Odometry
from std_msgs.msg import Float64
from dynamic_reconfigure.server import Server
from robot_controll.cfg import PIDConfig
import math
import time

class PIDController:
    def __init__(self):
        rospy.init_node('pid_controller_node', anonymous=True)

        # PID 초기값
        self.Kp_angle = 6.0
        self.Kd_angle = -0.75
        self.Kp_speed = 0.3
        self.Kp_steering = 0.95
        self.Kd_steering = -0.008

        self.setpoint_angle = 0.0
        self.prev_pitch_angle = 0.0
        self.setpoint_speed = 0.0
        self.current_speed = 0.0
        self.setpoint_steering = 0.0
        self.prev_roll_angle = 0.0

        self.last_cmd_time = time.time()
        self.cmd_timeout = 1.0
        self.prev_time = time.time()

        # setpoint_angle 발행
        self.setpoint_angle_pub = rospy.Publisher('/setpoint_angle', Float64, queue_size=10)

        # ROS 구독/발행
        self.imu_subscriber = rospy.Subscriber('/imu', Imu, self.imu_callback)
        self.before_vel_subscriber = rospy.Subscriber('/before_vel', Twist, self.before_vel_callback)
        self.odom_subscriber = rospy.Subscriber('/odom', Odometry, self.odom_callback)
        self.cmd_vel_publisher = rospy.Publisher('/cmd_vel', Twist, queue_size=10)

        self.rate = rospy.Rate(200)

        # dynamic_reconfigure 서버
        self.srv = Server(PIDConfig, self.reconfigure_callback)

    def reconfigure_callback(self, config, level):
        self.Kp_angle = config["Kp_angle"]
        self.Kd_angle = config["Kd_angle"]
        self.Kp_speed = config["Kp_speed"]
        self.Kp_steering = config["Kp_steering"]
        self.Kd_steering = config["Kd_steering"]
        return config

    def imu_callback(self, msg):
        current_pitch_angle = self.get_pitch(msg)
        current_roll_angle = self.get_roll(msg)
        current_time = rospy.Time.now().to_sec()
        dt = current_time - self.prev_time
        if dt <= 0:
            dt = 1e-3

        pitch_rate = (current_pitch_angle - self.prev_pitch_angle) / dt
        roll_rate = (current_roll_angle - self.prev_roll_angle) / dt
        self.prev_time = current_time

        # 넘어짐 체크
        if abs(current_pitch_angle) > math.radians(60):
            cmd = Twist()
            cmd.linear.x = 0.0
            cmd.angular.z = 0.0
            self.cmd_vel_publisher.publish(cmd)
            return

        # PID 계산
        speed_error = self.setpoint_speed - self.current_speed
        self.setpoint_angle = self.Kp_speed * speed_error
        # setpoint_angle 제한
        if abs(self.setpoint_angle) > math.radians(50):
            self.setpoint_angle = math.radians(50)
            
        angle_error = self.setpoint_angle - current_pitch_angle
        output_angle = self.Kp_angle * angle_error + self.Kd_angle * pitch_rate
        output_steering = - (self.Kp_steering * self.setpoint_steering + self.Kd_steering * roll_rate)

        cmd = Twist()
        cmd.linear.x = output_angle
        cmd.angular.z = output_steering
        self.cmd_vel_publisher.publish(cmd)

        # setpoint_angle 발행
        self.setpoint_angle_pub.publish(Float64(self.setpoint_angle))

        self.prev_pitch_angle = current_pitch_angle
        self.prev_roll_angle = current_roll_angle

        if time.time() - self.last_cmd_time > self.cmd_timeout:
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
            imu_msg.orientation.x, imu_msg.orientation.y,
            imu_msg.orientation.z, imu_msg.orientation.w
        )
        return pitch

    def get_roll(self, imu_msg):
        roll, _, _ = self.quaternion_to_euler(
            imu_msg.orientation.x, imu_msg.orientation.y,
            imu_msg.orientation.z, imu_msg.orientation.w
        )
        return roll

    def quaternion_to_euler(self, x, y, z, w):
        t0 = +2.0 * (w * x + y * z)
        t1 = +1.0 - 2.0 * (x * x + y * y)
        X = math.atan2(t0, t1)
        t2 = +2.0 * (w * y - z * x)
        t2 = max(min(t2, 1.0), -1.0)
        Y = math.asin(t2)
        t3 = +2.0 * (w * z + x * y)
        t4 = +1.0 - 2.0 * (y * y + z * z)
        Z = math.atan2(t3, t4)
        return X, Y, Z

    def run(self):
        while not rospy.is_shutdown():
            self.rate.sleep()


if __name__ == '__main__':
    controller = PIDController()
    controller.run()

