#include <ros.h>
#include <geometry_msgs/Twist.h>

ros::NodeHandle nh;
geometry_msgs::Twist cmd_vel_msg;

ros::Publisher cmd_vel_pub("cmd_vel", &cmd_vel_msg);

int steering_pin = 3;  // Steering 
int throttle_pin = 2;   // Throttle 
int aux_pin = 18;        

unsigned long steering_pwm = 0;
unsigned long throttle_pwm = 0;
unsigned long aux_pwm = 0;

void setup() {
  pinMode(steering_pin, INPUT);
  pinMode(throttle_pin, INPUT);
  pinMode(aux_pin, INPUT);

  nh.initNode();
  nh.advertise(cmd_vel_pub);  
}

void loop() {
  steering_pwm = pulseIn(steering_pin, HIGH);
  throttle_pwm = pulseIn(throttle_pin, HIGH);
  aux_pwm = pulseIn(aux_pin, HIGH);


  float linear_x = map(throttle_pwm, 1000, 2000, -100, 100) / 100.0;
  float angular_z = map(steering_pwm, 1000, 2000, -100, 100) / 100.0;


  cmd_vel_msg.linear.x = linear_x;
  cmd_vel_msg.angular.z = angular_z;


  cmd_vel_pub.publish(&cmd_vel_msg);

  nh.spinOnce();
  delay(50);  
}
