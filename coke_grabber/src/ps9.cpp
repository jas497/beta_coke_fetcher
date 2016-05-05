// example_object_grabber_action_client: 
// illustrates use of object_grabber action server called "objectGrabberActionServer"

#include <ros/ros.h>
#include <Eigen/Eigen>  //  for the Eigen library
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>
#include <object_recognition_msgs/RecognizedObjectArray.h>
#include <object_recognition_msgs/TableArray.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>  //  transform listener headers
#include <tf/transform_broadcaster.h>

std::string coke_id = "";
double coke_confidence = 0;
geometry_msgs::PoseStamped coke_pose;
bool firstCB = false;

void objectCallback(const object_recognition_msgs::RecognizedObjectArray objects_msg) {
    double confident = 0;
    int id = -1;
    //ROS_WARN("Total %d objects found", (int)objects_msg.objects.size());
    //ROS_INFO("Frame_id 1: %s", objects_msg.header.frame_id.c_str());

    if (firstCB == false && (int)objects_msg.objects.size() == 1) {
        coke_id.assign(objects_msg.objects[0].type.key.c_str());
        firstCB = true;
    }
    for (int i = 0; i < objects_msg.objects.size(); ++i) {
        if (coke_id.compare(objects_msg.objects[i].type.key.c_str()) == 0) {
            if (objects_msg.objects[i].confidence > confident) {
                confident = objects_msg.objects[i].confidence;
                id = i;
            }
        }
    }
    if (id >= 0) {
        coke_pose.pose = objects_msg.objects[id].pose.pose.pose;
        coke_confidence = objects_msg.objects[id].confidence;
    } else {
        confident = 0;
    }
}

void planeCallback(const object_recognition_msgs::TableArray plane_msg) {
    ROS_WARN("Total %d planes found", (int)plane_msg.tables.size());
    /*
	  for (int i = 0; i < plane_msg.tables.size(); ++i) {
	  ROS_INFO("Plane %d: plane pose x is: %f", i+1, plane_msg.tables[i].pose.position.x);
	  ROS_INFO("Plane %d: plane pose y is: %f", i+1, plane_msg.tables[i].pose.position.y);
	  ROS_INFO("Plane %d: plane pose z is: %f", i+1, plane_msg.tables[i].pose.position.z);
	  ROS_INFO("---------------------------------");
	  }*/

}

int main(int argc, char** argv) {
    ros::init(argc, argv, "coke_grabber_action_client"); // name this node
    ros::NodeHandle nh; //standard ros node handle    
    ros::Subscriber object_sub = nh.subscribe("/recognized_object_array", 1, &objectCallback);
    ros::Subscriber plane_sub = nh.subscribe("/table_array", 1, &planeCallback);
    
    coke_pose.header.frame_id = "camera_depth_optical_frame";

    geometry_msgs::PoseStamped transformed_pose;
    tf::TransformListener tf_listener; //start a transform listener

    ros::Duration loop_timer(3.0);

    while (ros::ok()) {
        if (coke_confidence > 0.8) {
			ROS_WARN("COKE!!!");
			
			ROS_INFO("Best Similarity = %f ", coke_confidence);
			ROS_INFO("pose x is: %f", coke_pose.pose.position.x);
			ROS_INFO("pose y is: %f", coke_pose.pose.position.y);
			ROS_INFO("pose z is: %f", coke_pose.pose.position.z);
			ROS_INFO("pose quat is: (%f,%f,%f,%f)",
					 coke_pose.pose.orientation.x,
					 coke_pose.pose.orientation.y,
					 coke_pose.pose.orientation.z,
					 coke_pose.pose.orientation.w);

			bool tferr = true;
			while (tferr) {
				tferr = false;
				try {
					tf_listener.transformPose("torso", coke_pose, transformed_pose);
				} catch (tf::TransformException &exception) {
					ROS_ERROR("%s", exception.what());
					tferr = true;
					ros::Duration(0.1).sleep(); // sleep for half a second
					ros::spinOnce();
				}
			}

			ROS_INFO("transformed coke pose x is: %f", transformed_pose.pose.position.x);
			ROS_INFO("transformed coke pose y is: %f", transformed_pose.pose.position.y);
			ROS_INFO("transformed coke pose z is: %f", transformed_pose.pose.position.z);
			ROS_INFO("pose quat is: (%f,%f,%f,%f)",
					 transformed_pose.pose.orientation.x,
					 transformed_pose.pose.orientation.y,
					 transformed_pose.pose.orientation.z,
					 transformed_pose.pose.orientation.w);
		} else {
			ROS_WARN("no coke :(");
		}
        ros::spinOnce();
        loop_timer.sleep();
    }
    return 0;
}

