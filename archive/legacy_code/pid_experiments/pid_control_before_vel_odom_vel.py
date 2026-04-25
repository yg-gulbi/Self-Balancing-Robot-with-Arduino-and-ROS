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
        self.Kp_angle = 20      
        self.Kd_angle = 0.008    
        self.Kp_speed = 0.03         
        self.Kp_steering = 0.0   
        self.Kd_steering = 0.0  
        self.max_output = 500
        self.ang_max = 0.3
        # PID 관련 변수
        self.setpoint_angle = 0.0   
        self.prev_error_angle = 0.0
        self.integral_angle = 0.0

        self.setpoint_speed = 0.0   
        self.prev_error_speed = 0.0
        self.integral_speed = 0.0
        self.current_speed = 0.0
        
        self.setpoint_steering = 0.0  
        self.prev_error_steering = 0.0

        # 마지막 명령 수신 시간 기록 및 타임아웃 설정
        self.last_cmd_time = time.time()
        self.cmd_timeout = 1.0  
     
        # 주제 구독 및 발행
        self.imu_subscriber = rospy.Subscriber('/imu', Imu, self.imu_callback)
        self.cmd_vel_publisher = rospy.Publisher('/cmd_vel', Twist, queue_size=10)        
        self.before_vel_subscriber = rospy.Subscriber('/before_vel', Twist, self.before_vel_callback)

        # 피치값과 각속도를 퍼블리시하기 위한 퍼블리셔 추가
        self.angular_velocity_publisher = rospy.Publisher('/angular_velocity', Float32, queue_size=10)  # 각속도 퍼블리셔
        
        # 로그 출력 주기 설정
        self.last_log_time = rospy.get_time()
        self.log_interval = 0.5  # 0.5초
  
        self.rate = rospy.Rate(100)  # 주기 20Hz
        
        self.current_steering = 0.0
        
                # 이전 시간 기록
        self.prev_time = rospy.get_time()
    def imu_callback(self, msg):
        # IMU에서 각도 읽어오기 (pitch)
        current_angle = self.get_pitch(msg)
        current_gyro_x = msg.angular_velocity.x  # IMU에서 피치 각속도 가져오기 (자이로 X축)
        current_gyro_z = msg.angular_velocity.z   # IMU에서 각속도 가져오기 (자이로 z축)
        # 선형 가속도 (x축을 사용)
        linear_acceleration_x = msg.linear_acceleration.x
        # 현재 시간과 이전 시간의 차이를 계산하여 속도 업데이트
        current_time = rospy.get_time()
        dt = current_time - self.prev_time

        # 가속도를 적분하여 선형속도 계산 (단위: m/s)
        self.current_speed += linear_acceleration_x * dt
        
        if abs(current_angle) > math.radians(40):  
            cmd = Twist()
            cmd.linear.x = 0.0
            cmd.angular.z = 0.0
            self.cmd_vel_publisher.publish(cmd)
           # rospy.loginfo("pitch angle exceeded 40 degrees. stopping the robot.")
            return
            
        # 각도 오차 계산
        # 선속도 오차 계산
        speed_error = self.setpoint_speed - self.current_speed 
                # PID 제어 계산 (속도)
        output_linear = max(min(speed_error, self.ang_max) , -self.ang_max)        
        self.setpoint_angle = self.Kp_speed * speed_error      
          
        current_time = rospy.get_time()
        if current_time - self.last_log_time >= self.log_interval:
            rospy.loginfo(f"My variable value: {self.setpoint_angle}")
            rospy.loginfo(f"My acelation: {linear_acceleration_x}")
            rospy.loginfo(f"My current speed: {self.current_speed}")
            rospy.loginfo(f"My current angle: {current_angle}")
            self.last_log_time = current_time
            
        angle_error = self.setpoint_angle - current_angle
        angle_derivative = current_gyro_x 


        # PID 제어 계산 (각도)
        output_angle = (self.Kp_angle * angle_error +
                        self.Kd_angle * angle_derivative)
        
        # STEERING 오차 계산
        steering_error = self.setpoint_steering
        steering_derivative = current_gyro_z      # 자이로 각속도를 사용

        # PID 제어 계산 (STEERING)
        output_steering = (self.Kp_steering * steering_error +
                           self.Kd_steering * steering_derivative)
        
        output_linear = max(min(output_angle, self.max_output) , -self.max_output)
        output_z = max(min(-output_steering , self.max_output) , -self.max_output)

        # Twist 메시지 발행
        cmd = Twist()
        cmd.linear.x = output_linear  
        cmd.angular.z = output_z         
        self.cmd_vel_publisher.publish(cmd)

        # 각속도 퍼블리시
        angular_velocity_msg = Float32()
        angular_velocity_msg.data = current_gyro_x  # 현재 각속도
        self.angular_velocity_publisher.publish(angular_velocity_msg)

        # 이전 오차 업데이트
        self.prev_error_angle = current_gyro_x  # 현재 피치 각속도로 업데이트
        self.prev_error_speed = speed_error
        self.prev_error_steering = steering_error

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


    def get_pitch(self, imu_msg):
        # IMU 메시지에서 pitch 각도 추출
        _, pitch, _ = self.quaternion_to_euler(
            imu_msg.orientation.x,
            imu_msg.orientation.y,
            imu_msg.orientation.z,
            imu_msg.orientation.w
        )
        return pitch

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

