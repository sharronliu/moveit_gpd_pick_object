#include <iostream>
#include <stdio.h>

#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit/collision_detection/collision_matrix.h>

#include <moveit_msgs/CollisionObject.h>
#include <moveit_msgs/AttachedCollisionObject.h>
#include <moveit_msgs/ApplyPlanningScene.h>
#include <moveit_msgs/Grasp.h>
#include <moveit_msgs/GetPlanningScene.h>
#include <moveit_msgs/PickupActionResult.h>

#include <manipulation_msgs/GraspPlanning.h>

class PickObject
{
public:
  moveit::planning_interface::MoveGroupInterface arm;
  moveit::planning_interface::MoveGroupInterface gripper;
  moveit::planning_interface::PlanningSceneInterface psi;

  PickObject() : arm("arm"), gripper("gripper")
  {
    ros::NodeHandle pnh("~");
    ros::NodeHandle nh;
  }

  bool executePick()
  {
    arm.setPlanningTime(20.0);
    return arm.planGraspsAndPick();
  }

  bool executePick(moveit_msgs::CollisionObject &object)
  {
    arm.setPlanningTime(20.0);
    arm.setSupportSurfaceName("table");
    return arm.planGraspsAndPick(object);
  }
private:
  void jointValuesToJointTrajectory(std::map<std::string, double> target_values, trajectory_msgs::JointTrajectory &grasp_pose)
  {
    grasp_pose.joint_names.reserve(target_values.size());
    grasp_pose.points.resize(1);
    grasp_pose.points[0].positions.reserve(target_values.size());

    for(std::map<std::string, double>::iterator it = target_values.begin(); it != target_values.end(); ++it)
    {
      grasp_pose.joint_names.push_back(it->first);
      grasp_pose.points[0].positions.push_back(it->second);
    }
  }
};

int main(int argc, char **argv)
{
  ros::init(argc, argv, "pick_object");
  ros::AsyncSpinner spinner(1);
  spinner.start();

  PickObject po;

  bool success = false;

  ROS_INFO("Picking Object");
  while(ros::ok())
  {
    if(success)
    {
      po.arm.setNamedTarget("extended");
      while(!po.arm.move())
        ROS_ERROR("moving to extended pose failed.");
      po.gripper.setNamedTarget("open");
      while(!po.gripper.move())
        ROS_ERROR("opening gripper failed.");
      po.arm.setNamedTarget("home");
      while(!po.arm.move())
        ROS_ERROR("moving home failed.");
      ros::Duration(5).sleep();
      success = false;
    }
    success = po.executePick();
    if(!success)
      ROS_ERROR("picking failed");
  }

  return 0;
}

