#!/usr/bin/env python3

import rospy
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Imu
from nav_msgs.msg import Odometry
from std_msgs.msg import Float32
import math
import time


class PIDController:
    def __init__(self):
        rospy.init_node('pid_controler', anonymous=True)

        # PID 파라미터
        self.Kp_angle = 8.0       #   3.5 # 5     #    8 #  
        self.Kd_angle = -1.45     # -0.08 # -0.35 # -1.5 #
        self.Kp_speed = 0.4               #0.3#0.4  
        self.Kp_steering = 0.8   
        self.Kd_steering = -0.008  
        self.max_output = 500

        # PID 관련 변수
        self.setpoint_angle = 0.0   
        self.prev_pitch_angle = 0.0
        self.setpoint_speed = 0.0   
        self.current_speed = 0.0 
        self.setpoint_steering = 0.0  
        self.prev_roll_angle = 0.0

        # 마지막 명령 수신 시간 기록 및 타임아웃 설정
        self.last_cmd_time = time.time()
        self.cmd_timeout = 1.0  
        # 이전 시간 초기화
        self.prev_time = time.time()

        # 주제 구독 및 발행
        self.imu_subscriber = rospy.Subscriber('/imu', Imu, self.imu_callback)
        self.before_vel_subscriber = rospy.Subscriber('/before_vel', Twist, self.before_vel_callback)
        self.odom_subscriber = rospy.Subscriber('/odom', Odometry, self.odom_callback)
        
        self.cmd_vel_publisher = rospy.Publisher('/cmd_vel', Twist, queue_size=10) 
        
        # 로그 출력 주기 설정
        self.last_log_time = rospy.get_time()
        self.log_interval = 1  # 0.5초
        
        self.rate = rospy.Rate(200)  # 주기 20Hz

    def imu_callback(self, msg):
        # IMU에서 각도 읽어오기 (pitch,roll)
        current_pitch_angle = self.get_pitch(msg)
        current_roll_angle = self.get_roll(msg)
        
        # 각속도 계산
        current_time = time.time()      
        dt = current_time - self.prev_time        
        pitch_rate = (current_pitch_angle - self.prev_pitch_angle) / dt       
        roll_rate = (current_roll_angle - self.prev_roll_angle) / dt
          
         #40도 넘어갈 시 정지 
        if abs(current_pitch_angle) > math.radians(40):  
            cmd = Twist()
            cmd.linear.x = 0.0
            cmd.angular.z = 0.0
            self.cmd_vel_publisher.publish(cmd)
            #rospy.loginfo("pitch angle exceeded 40 degrees. stopping the robot.")
            return
            
        # PID 계산 (속도)
        speed_error = self.setpoint_speed - self.current_speed 
        self.setpoint_angle = self.Kp_speed * speed_error    
        self.setpoint_angle = max(min(self.setpoint_angle,math.radians(30)),-math.radians(30))
        
        # PID 계산 (각도)             
        angle_error = self.setpoint_angle - 0.015 - current_pitch_angle  
        output_angle = (self.Kp_angle * angle_error + self.Kd_angle * pitch_rate)

        # PID 계산 (STEERING)
        output_steering = - (self.Kp_steering * self.setpoint_steering + self.Kd_steering * roll_rate)
        
        # Twist 메시지 발행
        cmd = Twist()
        cmd.linear.x = output_angle  
        cmd.angular.z = output_steering         
        self.cmd_vel_publisher.publish(cmd)

        # 이전 오차 업데이트
        self.prev_pitch_angle = current_pitch_angle
        self.prev_roll_angle = current_roll_angle
        self.prev_time = current_time  
                
        if current_time - self.last_log_time >= self.log_interval:
            rospy.loginfo(f" My setpoint_angle: {self.setpoint_angle}")
            rospy.loginfo(f"  My current angle: {current_pitch_angle}")
            rospy.loginfo(f"  My current speed: {self.current_speed}")
            rospy.loginfo(f"        pitch_rate: {pitch_rate}")  
            rospy.loginfo(f"      output_angle: {output_angle}")
            self.last_log_time = current_time  
               
        # 메시지 타임아웃 확인 및 설정 초기화
        if time.time() - self.last_cmd_time > self.cmd_timeout:
            self.setpoint_speed = 0.0
            self.setpoint_steering = 0.0

    def before_vel_callback(self, msg):
        # CMD_MSG에서 목표 선속도와 목표 회전 속도 업데이트
        self.setpoint_speed = msg.linear.x     
        self.setpoint_steering = msg.angular.z 

        # 마지막 명령 수신 시간 업데이트
        self.last_cmd_time = time.time()

    def odom_callback(self, msg):
        # /odom에서 현재 선속도 읽어오기
        self.current_speed = msg.twist.twist.linear.x


    def get_pitch(self, imu_msg):
        # IMU 메시지에서 pitch 각도 추출
        _, pitch, _ = self.quaternion_to_euler(
            imu_msg.orientation.x,
            imu_msg.orientation.y,
            imu_msg.orientation.z,
            imu_msg.orientation.w
        )
        return pitch
    
    def get_roll(self, imu_msg):
        # IMU 메시지에서 pitch 각도 추출
        roll, _, _ = self.quaternion_to_euler(
            imu_msg.orientation.x,
            imu_msg.orientation.y,
            imu_msg.orientation.z,
            imu_msg.orientation.w
        )
        return roll    

    def quaternion_to_euler(self, x, y, z, w):
        # 쿼터니언을 오일러 각도로 변환하는 함수
        t0 = +2.0 * (w * x + y * z)
        t1 = +1.0 - 2.0 * (x * x + y * y)
        X = math.atan2(t0, t1)

        t2 = +2.0 * (w * y - z * x)
        t2 = +1.0 if t2 > +1.0 else t2
        t2 = -1.0 if t2 < -1.0 else t2
        Y = math.asin(t2)

        t3 = +2.0 * (w * z + x * y)
        t4 = +1.0 - 2.0 * (y * y + z * z)
        Z = math.atan2(t3, t4)

        return X, Y, Z  # roll, pitch, yaw

    def run(self):
        while not rospy.is_shutdown():
            self.rate.sleep()


if __name__ == '__main__':
    controller = PIDController()
    controller.run()

