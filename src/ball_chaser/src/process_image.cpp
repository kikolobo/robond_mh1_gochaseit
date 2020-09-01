#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;


// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;


    client.call(srv);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int white_pixel = 255;

    int img_center_area = img.step / 3;
    int img_right_area = img.step - img_center_area;
    int total_pixels = img.height * img.step;
   
    bool is_right = false;
    bool is_center = false;
    bool is_left = false;
    bool is_notInView = true;
    
    for (int i = 0; i < total_pixels; i=i+3) {
        int pixelValue = (img.data[i] + img.data[i+1] + img.data[i+3]) / 3;

        if (pixelValue == white_pixel) {
		is_notInView = false;
		int col = i % img.step;

		if (col <= img_center_area) { is_left = true; ROS_INFO_STREAM("[L]"); }
		if (col >= img_right_area) { is_right = true; ROS_INFO_STREAM("[R]");}
		if (is_left != true && is_right != true) { is_center = true; ROS_INFO_STREAM("[M]");} 
		break;     
        }
    }

   
    if (is_notInView == true) {
 	drive_robot(0,0);
    } else {
	if (is_center == true) { drive_robot(0.1, 0.0); }
	if (is_left == true) { drive_robot(0.0, 0.1); }
	if (is_right == true) { drive_robot(0.0, -0.1); }

    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
