/****************************************************************
 *
 * Copyright (c) 2010
 *
 * Fraunhofer Institute for Manufacturing Engineering	
 * and Automation (IPA)
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Project name: care-o-bot
 * ROS stack name: cob_driver
 * ROS package name: cob_camera_sensors
 *								
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *			
 * Author: Richard Bormann, email:richard.bormann@ipa.fhg.de
 * Supervised by: Richard Bormann, email:richard.bormann@ipa.fhg.de
 *
 * Date of creation: May 2011
 * ToDo:
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Fraunhofer Institute for Manufacturing 
 *       Engineering and Automation (IPA) nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License LGPL for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License LGPL along with this program. 
 * If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************/

//##################
//#### includes ####

// ROS includes
#include <ros/ros.h>
//#include <nodelet/nodelet.h>

// ROS message includes
//#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <tf/transform_listener.h>

// topics
//#include <message_filters/subscriber.h>
//#include <message_filters/synchronizer.h>
//#include <message_filters/sync_policies/approximate_time.h>
//#include <image_transport/image_transport.h>
//#include <image_transport/subscriber_filter.h>

// opencv
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

// point cloud
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>

// Plugins
//#include <pluginlib/class_list_macros.h>

// System
#include <iostream>

#include <cob_vision_utils/GlobalDefines.h>

//namespace ipa_CameraSensors
//{
class CobKinectImageFlip
{
protected:
	ros::Subscriber point_cloud_sub_;
	ros::Publisher point_cloud_pub_; ///< Point cloud output topic
	//message_filters::Subscriber<sensor_msgs::PointCloud2> point_cloud_sub_;	///< Point cloud input topic
	//image_transport::ImageTransport* it_;
	//image_transport::SubscriberFilter color_camera_image_sub_;	///< Color camera image input topic
	//image_transport::Publisher color_camera_image_pub_;		///< Color camera image output topic
	//message_filters::Synchronizer<message_filters::sync_policies::ApproximateTime<sensor_msgs::PointCloud2, sensor_msgs::Image> >* sync_pointcloud_;	///< synchronizer for input data
	//message_filters::Connection sync_pointcloud_callback_connection_;

	tf::TransformListener* transform_listener_;

	ros::NodeHandle node_handle_; ///< ROS node handle

public:

	CobKinectImageFlip(ros::NodeHandle nh)
	{
		node_handle_ = nh;

		//ros::get_environment_variable();
		//it_ = 0;
		//sync_pointcloud_ = 0;
		transform_listener_ = 0;

//		it_ = new image_transport::ImageTransport(node_handle_);
//		sync_pointcloud_ = new message_filters::Synchronizer<message_filters::sync_policies::ApproximateTime<sensor_msgs::PointCloud2, sensor_msgs::Image>>(3);//(2);  //this parameter is the queue size for comparing time stanps of messages (might work badly if set to 1)
//		color_camera_image_pub_ = it_->advertise("rgb/upright/image_color", 1);
		point_cloud_sub_ = node_handle_.subscribe<sensor_msgs::PointCloud2>("pointcloud_in", 1, &CobKinectImageFlip::inputCallback, this);
		point_cloud_pub_ = node_handle_.advertise<sensor_msgs::PointCloud2>("pointcloud_out", 1);

		transform_listener_ = new tf::TransformListener(node_handle_);

		// initializations
		init();

		std::cout << "CobKinectImageFlip initilized.\n";
	}

	~CobKinectImageFlip()
	{
//		if (it_ != 0)
//			delete it_;
//		if (sync_pointcloud_ != 0)
//			delete sync_pointcloud_;
		if (transform_listener_ != 0)
			delete transform_listener_;
	}

	unsigned long init()
	{
		//color_camera_image_sub_.subscribe(*it_, "colorimage", 1);
		//point_cloud_sub_.subscribe(node_handle_, "pointcloud", 1);

		//sync_pointcloud_->connectInput(point_cloud_sub_, color_camera_image_sub_);
		//sync_pointcloud_callback_connection_ = sync_pointcloud_->registerCallback(boost::bind(&CobKinectImageFlipNodelet::inputCallback, this, _1, _2));

		return ipa_Utils::RET_OK;
	}

//	unsigned long convertColorImageMessageToMat(const sensor_msgs::Image::ConstPtr& color_image_msg, cv_bridge::CvImageConstPtr& color_image_ptr, cv::Mat& color_image)
//	{
//		try
//		{
//			color_image_ptr = cv_bridge::toCvShare(color_image_msg, sensor_msgs::image_encodings::BGR8);
//		} catch (cv_bridge::Exception& e)
//		{
//			ROS_ERROR("PeopleDetection: cv_bridge exception: %s", e.what());
//			return ipa_Utils::RET_FAILED;
//		}
//		color_image = color_image_ptr->image;
//
//		return ipa_Utils::RET_OK;
//	}

	void inputCallback(const sensor_msgs::PointCloud2::ConstPtr& point_cloud_msg)
	{
		// check camera link orientation and decide whether image must be turned around

		bool turnAround = false;
		tf::StampedTransform transform;
		try
		{
			transform_listener_->lookupTransform("/base_link", "/head_cam3d_link", ros::Time(0), transform);
			btScalar roll, pitch, yaw;
			transform.getBasis().getRPY(roll, pitch, yaw, 1);
			if (roll > 0.0)
				turnAround = true;
			//      std::cout << "xyz: " << transform.getOrigin().getX() << " " << transform.getOrigin().getY() << " " << transform.getOrigin().getZ() << "\n";
			//      std::cout << "abcw: " << transform.getRotation().getX() << " " << transform.getRotation().getY() << " " << transform.getRotation().getZ() << " " << transform.getRotation().getW() << "\n";
			//      std::cout << "rpy: " << roll << " " << pitch << " " << yaw << "\n";
		} catch (tf::TransformException ex)
		{
			ROS_WARN("%s",ex.what());
		}

		if (turnAround == false)
		{
			// image upright
			//sensor_msgs::Image color_image_turned_msg = *color_image_msg;
			//color_image_turned_msg.header.stamp = ros::Time::now();
			//color_camera_image_pub_.publish(color_image_turned_msg);
			//sensor_msgs::PointCloud2::ConstPtr point_cloud_turned_msg = point_cloud_msg;
			point_cloud_pub_.publish(point_cloud_msg);
		}
		else
		{
			// image upside down
			// point cloud
			pcl::PointCloud<pcl::PointXYZRGB> point_cloud_src;
			pcl::fromROSMsg(*point_cloud_msg, point_cloud_src);

			pcl::PointCloud<pcl::PointXYZRGB>::Ptr point_cloud_turned(new pcl::PointCloud<pcl::PointXYZRGB>);
			point_cloud_turned->resize(point_cloud_src.height*point_cloud_src.width);
			int point_cloud_turned_counter = 0;
			for (int v = (int)point_cloud_src.height - 1; v >= 0; v--)
			{
				for (int u = (int)point_cloud_src.width - 1; u >= 0; u--)
				{
					point_cloud_turned->at(point_cloud_turned_counter) = point_cloud_src(u, v);
					point_cloud_turned_counter++;
				}
			}

			// publish turned data
			point_cloud_turned->header = point_cloud_msg->header;
			point_cloud_turned->height = point_cloud_msg->height;
			point_cloud_turned->width = point_cloud_msg->width;
			//point_cloud_turned->sensor_orientation_ = point_cloud_msg->sensor_orientation_;
			//point_cloud_turned->sensor_origin_ = point_cloud_msg->sensor_origin_;
			point_cloud_turned->is_dense = point_cloud_msg->is_dense;
			sensor_msgs::PointCloud2::Ptr point_cloud_turned_msg(new sensor_msgs::PointCloud2);
			pcl::toROSMsg(*point_cloud_turned, *point_cloud_turned_msg);
			//point_cloud_turned_msg->header.stamp = ros::Time::now();
			point_cloud_pub_.publish(point_cloud_turned_msg);

			//      cv::namedWindow("test");
			//      cv::imshow("test", color_image_turned);
			//      cv::waitKey(10);
		}
	}
};


//#######################
//#### main programm ####
int main(int argc, char** argv)
{
	// Initialize ROS, specify name of node
	ros::init(argc, argv, "cob_image_flip");

	// Create a handle for this node, initialize node
	ros::NodeHandle nh;

	// Create SensorMessageGatewayNode class instance
	CobKinectImageFlip cobKinectImageFlip(nh);

	ros::spin();

	return 0;
}

//}

//PLUGINLIB_DECLARE_CLASS(ipa_CameraSensors, CobKinectImageFlipNodelet, ipa_CameraSensors::CobKinectImageFlipNodelet, nodelet::Nodelet)
