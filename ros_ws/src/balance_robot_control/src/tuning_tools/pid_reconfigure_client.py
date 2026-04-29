#!/usr/bin/env python3

from dynamic_reconfigure.client import Client
import rospy

rospy.init_node("pid_client")

# PID 서버 노드 이름
client = Client("/pid_control")  

# PID 값 바꾸기
params = {
    "Kp_angle": 10.0,
    "Kd_angle": -0.8
}
client.update_configuration(params)

rospy.loginfo("PID 값 변경 완료")
