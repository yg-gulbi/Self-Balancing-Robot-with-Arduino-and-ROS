#!/usr/bin/env python3

import rospy
from std_srvs.srv import Trigger, TriggerResponse
import subprocess

# 런치 파일 실행 중지 상태를 저장할 변수
launch_process = None

def start_slam_launch_file(req):
    global launch_process
    if launch_process is None:  # 이미 실행 중이지 않을 때만 실행
        try:
            launch_process = subprocess.Popen(["roslaunch", "robot_ability", "robot_slam.launch"])
            return TriggerResponse(success=True, message="런치 파일 실행됨")
        except Exception as e:
            return TriggerResponse(success=False, message=f"실행 실패: {e}")
    else:
        return TriggerResponse(success=False, message="런치 파일이 이미 실행 중입니다.")

def start_navigation_launch_file(req):
    global launch_process
    if launch_process is None:  # 이미 실행 중이지 않을 때만 실행
        try:
            launch_process = subprocess.Popen(["roslaunch", "robot_ability", "robot_navigation.launch"])
            return TriggerResponse(success=True, message="런치 파일 실행됨")
        except Exception as e:
            return TriggerResponse(success=False, message=f"실행 실패: {e}")
    else:
        return TriggerResponse(success=False, message="런치 파일이 이미 실행 중입니다.")

def stop_launch_file(req):
    global launch_process
    if launch_process is not None:  # 실행 중인 프로세스가 있을 때만 종료
        try:
            launch_process.terminate()
            launch_process = None
            return TriggerResponse(success=True, message="런치 파일 종료됨")
        except Exception as e:
            return TriggerResponse(success=False, message=f"종료 실패: {e}")
    else:
        return TriggerResponse(success=False, message="실행 중인 런치 파일이 없습니다.")

if __name__ == "__main__":
    rospy.init_node("launch_service_node")
    rospy.Service("start_slam_launch", Trigger, start_slam_launch_file)
    rospy.Service("start_navigation_launch", Trigger, start_navigation_launch_file)
    rospy.Service("stop_launch", Trigger, stop_launch_file)
    rospy.loginfo("런치 파일 제어 서비스가 준비되었습니다.")
    rospy.spin()
