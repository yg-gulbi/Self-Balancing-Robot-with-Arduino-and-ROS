#!/usr/bin/env python3
import rospy
from std_srvs.srv import Empty
import time

def reset_and_restart_gazebo():
    rospy.init_node('gazebo_reset_node', anonymous=True)

    # 서비스 연결 대기
    rospy.wait_for_service('/gazebo/reset_world')
    rospy.wait_for_service('/gazebo/pause_physics')
    rospy.wait_for_service('/gazebo/unpause_physics')

    # 서비스 프록시 생성
    reset_world = rospy.ServiceProxy('/gazebo/reset_world', Empty)
    pause_physics = rospy.ServiceProxy('/gazebo/pause_physics', Empty)
    unpause_physics = rospy.ServiceProxy('/gazebo/unpause_physics', Empty)

    try:
        # 1. 시뮬레이션 일시정지
        pause_physics()
        rospy.loginfo("Gazebo simulation paused")

        # 2. 월드 리셋
        reset_world()
        rospy.loginfo("Gazebo world reset")

        # 3. 잠깐 기다리기 (1초)
        time.sleep(1)

        # 4. 시뮬레이션 재개
        unpause_physics()
        rospy.loginfo("Gazebo simulation restarted")

    except rospy.ServiceException as e:
        rospy.logerr(f"Service call failed: {e}")

if __name__ == "__main__":
    reset_and_restart_gazebo()
