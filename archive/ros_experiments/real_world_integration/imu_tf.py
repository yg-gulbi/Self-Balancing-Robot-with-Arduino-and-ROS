#!/usr/bin/env python3

import rospy
import tf2_ros
import geometry_msgs.msg
import sensor_msgs.msg
import tf.transformations as tft

def extract_pitch_from_quaternion(orientation):
    # 쿼터니언을 오일러 각으로 변환
    euler = tft.euler_from_quaternion([orientation.x, orientation.y, orientation.z, orientation.w])

    # y축 기울기(pitch)만 반환
    pitch = euler[1]

    # pitch를 이용해 새로운 쿼터니언 생성 (roll과 yaw는 0)
    new_quaternion = tft.quaternion_from_euler(0.0, pitch, 0.0)

    return new_quaternion

def imu_callback(msg):
    # IMU 메시지에서 회전 정보(Quaternion)를 pitch만 포함한 쿼터니언으로 변환
    modified_quaternion = extract_pitch_from_quaternion(msg.orientation)

    # TF 변환 생성
    t = geometry_msgs.msg.TransformStamped()

    t.header.stamp = rospy.Time.now()
    t.header.frame_id = "base_footprint"
    t.child_frame_id = "base_link"

    # 변환된 쿼터니언을 변환에 적용
    t.transform.rotation.x = modified_quaternion[0]
    t.transform.rotation.y = modified_quaternion[1]
    t.transform.rotation.z = modified_quaternion[2]
    t.transform.rotation.w = modified_quaternion[3]

    # tf 브로드캐스트
    br.sendTransform(t)

if __name__ == '__main__':
    rospy.init_node('dynamic_tf2_broadcaster')

    # TF 브로드캐스터 객체 생성
    br = tf2_ros.TransformBroadcaster()

    # IMU 데이터 구독
    rospy.Subscriber("/imu", sensor_msgs.msg.Imu, imu_callback)

    rospy.spin()
