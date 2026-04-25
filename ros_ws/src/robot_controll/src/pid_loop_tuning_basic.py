#!/usr/bin/env python3
import rospy
from std_srvs.srv import Empty
from geometry_msgs.msg import Twist
from dynamic_reconfigure.client import Client
import time

def reset_and_move_forward(pid_params, loop_idx):
    rospy.loginfo(f"Loop {loop_idx+1}/100: starting with PID {pid_params}")

    # PID 파라미터 변경
    client = Client("/pid_controll", timeout=5)  # PID 노드 이름 확인
    client.update_configuration(pid_params)

    # 서비스 프록시 생성
    reset_world = rospy.ServiceProxy('/gazebo/reset_world', Empty)
    pause_physics = rospy.ServiceProxy('/gazebo/pause_physics', Empty)
    unpause_physics = rospy.ServiceProxy('/gazebo/unpause_physics', Empty)

    # /before_vel 퍼블리셔 생성
    pub_vel = rospy.Publisher('/before_vel', Twist, queue_size=10)
    vel_msg = Twist()
    vel_msg.linear.x = 0.5
    vel_msg.angular.z = 0.0

    rospy.sleep(0.5)  # 퍼블리셔 안정화

    try:
        pause_physics()
        rospy.loginfo("Gazebo simulation paused")

        reset_world()
        rospy.loginfo("Gazebo world reset")

        time.sleep(1)

        unpause_physics()
        rospy.loginfo("Gazebo simulation restarted")

        # 3초 전진
        start_time = time.time()
        while time.time() - start_time < 3.0:
            pub_vel.publish(vel_msg)
            rospy.sleep(0.05)

        # 정지
        vel_msg.linear.x = 0.0
        pub_vel.publish(vel_msg)
        rospy.loginfo("Robot stopped after moving forward")

    except rospy.ServiceException as e:
        rospy.logerr(f"Service call failed: {e}")


if __name__ == "__main__":
    rospy.init_node('gazebo_loop_node', anonymous=True)
    rospy.wait_for_service('/gazebo/reset_world')
    rospy.wait_for_service('/gazebo/pause_physics')
    rospy.wait_for_service('/gazebo/unpause_physics')

    # 단일 PID 값 설정
    pid_params = {"Kp_angle": 6.0, "Kd_angle": -0.75, "Kp_speed": 0.3}

    # 100번 반복
    for i in range(100):
        reset_and_move_forward(pid_params, i)
        rospy.sleep(3)  # 루프 사이 잠깐 대기

    rospy.loginfo("Finished 100 test loops")

