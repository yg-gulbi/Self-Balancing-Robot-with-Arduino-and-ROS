#!/usr/bin/env python3
import rospy
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Imu
from nav_msgs.msg import Odometry
from std_msgs.msg import Float64
from std_srvs.srv import Empty
from dynamic_reconfigure.client import Client
import time
import math
import numpy as np
from bayes_opt import BayesianOptimization

# ----------------------------
# 쿼터니언 → 오일러 변환
# ----------------------------
def quaternion_to_euler(x, y, z, w):
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + y * y)
    roll = math.atan2(t0, t1)

    t2 = +2.0 * (w * y - z * x)
    t2 = max(min(t2, 1.0), -1.0)
    pitch = math.asin(t2)

    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (y * y + z * z)
    yaw = math.atan2(t3, t4)

    return roll, pitch, yaw

# ----------------------------
# 단일 시뮬레이션 루프
# ----------------------------
def run_single_loop(pid_params):
    rospy.loginfo(f"Running loop with PID {pid_params}")

    # PID 업데이트
    client = Client("/pid_controll", timeout=5)
    client.update_configuration(pid_params)

    # Gazebo 서비스
    reset_world = rospy.ServiceProxy('/gazebo/reset_world', Empty)
    pause_physics = rospy.ServiceProxy('/gazebo/pause_physics', Empty)
    unpause_physics = rospy.ServiceProxy('/gazebo/unpause_physics', Empty)

    # 퍼블리셔
    pub_vel = rospy.Publisher('/before_vel', Twist, queue_size=10)
    vel_msg = Twist()
    rospy.sleep(0.5)  # 퍼블리셔 안정화

    # 기록용
    speed_history = []
    angular_history = []
    pitch_history = []
    setpoint_history = []
    fallen = False

    # ----------------------------
    # 콜백 함수
    # ----------------------------
    def odom_callback(msg):
        speed_history.append(msg.twist.twist.linear.x)
        angular_history.append(msg.twist.twist.angular.z)

    def imu_callback(msg):
        nonlocal fallen
        _, pitch, _ = quaternion_to_euler(
            msg.orientation.x, msg.orientation.y,
            msg.orientation.z, msg.orientation.w
        )
        pitch_history.append(pitch)
        if abs(pitch) > math.radians(40):
            fallen = True

    def setpoint_callback(msg):
        setpoint_history.append(msg.data)

    # ----------------------------
    # 구독자 등록
    # ----------------------------
    rospy.Subscriber('/odom', Odometry, odom_callback)
    rospy.Subscriber('/imu', Imu, imu_callback)
    rospy.Subscriber('/setpoint_angle', Float64, setpoint_callback)

    try:
        # 시뮬레이션 초기화
        pause_physics()
        reset_world()
        unpause_physics()
        
        # -----------------
        # 초기 정지 (3초)
        # -----------------
        vel_msg.linear.x = 0.0
        pub_vel.publish(vel_msg)
        stop_start = time.time()
        while time.time() - stop_start < 3.0:
            rospy.sleep(0.01)
            if fallen:
                break

        # -----------------
        # 전진 (3초)
        # -----------------
        vel_msg.linear.x = 0.5
        vel_msg.angular.z = 0.5
        start_time = time.time()
        while time.time() - start_time < 3.0:
            pub_vel.publish(vel_msg)
            rospy.sleep(0.01)
            if fallen:
                break

        # -----------------
        # 최종 정지 (5초)
        # -----------------
        vel_msg.linear.x = 0.0
        vel_msg.angular.z = 0.0
        pub_vel.publish(vel_msg)
        stop_start = time.time()
        while time.time() - stop_start < 5.0:
            rospy.sleep(0.01)
            if fallen:
                break

    except rospy.ServiceException as e:
        rospy.logerr(f"Service call failed: {e}")

    # ----------------------------
    # 데이터 검증
    # ----------------------------
    if fallen:
        rospy.logwarn("Robot fell → heavy penalty")
        return -100.0

    min_len = min(len(pitch_history), len(setpoint_history), len(speed_history), len(angular_history))
    if min_len == 0:
        rospy.logerr("No setpoint, pitch, or speed data → aborting program")
        rospy.signal_shutdown("Critical data missing")
        return

    pitch_array = np.array(pitch_history[:min_len])
    setpoint_array = np.array(setpoint_history[:min_len])
    speed_array = np.array(speed_history[:min_len])
    angular_z_array = np.array(angular_history[:min_len])

    # ----------------------------
    # 구간별 오차 계산
    # ----------------------------
    samples_per_phase = int(3.0 / 0.01)
    initial_stop_speeds = speed_array[:samples_per_phase]
    forward_speeds = speed_array[samples_per_phase:samples_per_phase*2]
    forward_ang_speeds = angular_z_array[samples_per_phase:samples_per_phase*2]

    samples_final = int(5.0 / 0.01)
    final_stop_speeds = speed_array[samples_per_phase*2 : samples_per_phase*2 + samples_final]

    # 목표값
    target_speed = 0.5
    target_angular = 0.5

    # 오차 계산
    tracking_error = np.mean(np.abs(forward_speeds - target_speed))
    stop_error = np.mean(np.abs(initial_stop_speeds)) + np.mean(np.abs(final_stop_speeds))
    angular_error = np.mean(np.abs(forward_ang_speeds - target_angular))
    balance_error = np.mean(np.abs(pitch_array - setpoint_array))

    # ----------------------------
    # 목적함수 (score)
    # ----------------------------
    score = - (1.0 * tracking_error + 1.0 * stop_error + 10.0 * balance_error + 1.0 * angular_error)

    # 로그
    rospy.loginfo(f"Tracking error: {tracking_error:.4f}")
    rospy.loginfo(f"Stop error: {stop_error:.4f}")
    rospy.loginfo(f"Balance error: {balance_error:.4f}")
    rospy.loginfo(f"Angular error: {angular_error:.4f}")
    rospy.loginfo(f"Loop finished. score={score:.3f}")

    return score

# ----------------------------
# 베이지안 최적화
# ----------------------------
def optimize_pid():
    rospy.init_node('pid_bayes_opt_node', anonymous=True)
    rospy.wait_for_service('/gazebo/reset_world')
    rospy.wait_for_service('/gazebo/pause_physics')
    rospy.wait_for_service('/gazebo/unpause_physics')

    def objective(Kp_angle, Kd_angle, Kp_speed):
        pid_params = {"Kp_angle": Kp_angle, "Kd_angle": Kd_angle, "Kp_speed": Kp_speed}
        return run_single_loop(pid_params)

    pbounds = {
        'Kp_angle': (2.0, 8.0),
        'Kd_angle': (-2, -0.05),
        'Kp_speed': (0.1, 0.5)
    }

    optimizer = BayesianOptimization(f=objective, pbounds=pbounds, verbose=2, random_state=1)

    # 초기 시드값
    initial_probes = [
        {'Kp_angle': 6.0, 'Kd_angle': -0.75, 'Kp_speed': 0.3},
        {'Kp_angle': 7.0, 'Kd_angle': -0.75, 'Kp_speed': 0.25}
    ]

    for probe in initial_probes:
        optimizer.probe(params=probe, lazy=False)

    optimizer.maximize(init_points=2, n_iter=100)
    rospy.loginfo(f"Best PID: {optimizer.max['params']}")

if __name__ == "__main__":
    optimize_pid()

