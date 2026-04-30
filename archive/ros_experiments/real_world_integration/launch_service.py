#!/usr/bin/env python3

import rospy
from std_srvs.srv import Trigger, TriggerResponse
import subprocess

# ?°ì¹˜ ?Œì¼ ?¤í–‰ ì¤‘ì? ?íƒœë¥??€?¥í•  ë³€??launch_process = None

def start_slam_launch_file(req):
    global launch_process
    if launch_process is None:  # ?´ë? ?¤í–‰ ì¤‘ì´ì§€ ?Šì„ ?Œë§Œ ?¤í–‰
        try:
            launch_process = subprocess.Popen(["roslaunch", "balance_robot_workflows", "robot_slam.launch"])
            return TriggerResponse(success=True, message="?°ì¹˜ ?Œì¼ ?¤í–‰??)
        except Exception as e:
            return TriggerResponse(success=False, message=f"?¤í–‰ ?¤íŒ¨: {e}")
    else:
        return TriggerResponse(success=False, message="?°ì¹˜ ?Œì¼???´ë? ?¤í–‰ ì¤‘ì…?ˆë‹¤.")

def start_navigation_launch_file(req):
    global launch_process
    if launch_process is None:  # ?´ë? ?¤í–‰ ì¤‘ì´ì§€ ?Šì„ ?Œë§Œ ?¤í–‰
        try:
            launch_process = subprocess.Popen(["roslaunch", "balance_robot_workflows", "robot_navigation.launch"])
            return TriggerResponse(success=True, message="?°ì¹˜ ?Œì¼ ?¤í–‰??)
        except Exception as e:
            return TriggerResponse(success=False, message=f"?¤í–‰ ?¤íŒ¨: {e}")
    else:
        return TriggerResponse(success=False, message="?°ì¹˜ ?Œì¼???´ë? ?¤í–‰ ì¤‘ì…?ˆë‹¤.")

def stop_launch_file(req):
    global launch_process
    if launch_process is not None:  # ?¤í–‰ ì¤‘ì¸ ?„ë¡œ?¸ìŠ¤ê°€ ?ˆì„ ?Œë§Œ ì¢…ë£Œ
        try:
            launch_process.terminate()
            launch_process = None
            return TriggerResponse(success=True, message="?°ì¹˜ ?Œì¼ ì¢…ë£Œ??)
        except Exception as e:
            return TriggerResponse(success=False, message=f"ì¢…ë£Œ ?¤íŒ¨: {e}")
    else:
        return TriggerResponse(success=False, message="?¤í–‰ ì¤‘ì¸ ?°ì¹˜ ?Œì¼???†ìŠµ?ˆë‹¤.")

if __name__ == "__main__":
    rospy.init_node("launch_service_node")
    rospy.Service("start_slam_launch", Trigger, start_slam_launch_file)
    rospy.Service("start_navigation_launch", Trigger, start_navigation_launch_file)
    rospy.Service("stop_launch", Trigger, stop_launch_file)
    rospy.loginfo("?°ì¹˜ ?Œì¼ ?œì–´ ?œë¹„?¤ê? ì¤€ë¹„ë˜?ˆìŠµ?ˆë‹¤.")
    rospy.spin()
