#!/usr/bin/env python3

import rospy
import tf2_ros
import geometry_msgs.msg
import nav_msgs.msg
import math

def odom_callback(msg):
    # odom 데이터가 들어올 때마다 tf 변환을 브로드캐스트
    t = geometry_msgs.msg.TransformStamped()

    t.header.stamp = rospy.Time.now()
    t.header.frame_id = "odom"
    t.child_frame_id = "camera_link"

    # odom 메시지에서 위치 정보 추출
    t.transform.translation.x = msg.pose.pose.position.x
    t.transform.translation.y = msg.pose.pose.position.y
    t.transform.translation.z = msg.pose.pose.position.z

    # odom 메시지에서 회전 정보(Quaternion) 추출
    t.transform.rotation = msg.pose.pose.orientation

    # tf 브로드캐스트
    br.sendTransform(t)

if __name__ == '__main__':
    rospy.init_node('dynamic_tf2_broadcaster')

    # TF 브로드캐스터 객체 생성
    br = tf2_ros.TransformBroadcaster()

    # odom 데이터 구독
    rospy.Subscriber("/odom", nav_msgs.msg.Odometry, odom_callback)

    rospy.spin()
