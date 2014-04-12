/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, University of Colorado, Boulder
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Univ of CO, Boulder nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

// Author: Dave Coleman
// Desc:   Filters grasps based on kinematic feasibility

#ifndef MOVEIT_SIMPLE_GRASPS__GRASP_FILTER_
#define MOVEIT_SIMPLE_GRASPS__GRASP_FILTER_

// ROS
#include <ros/ros.h>
//#include <tf/tf.h>
#include <tf_conversions/tf_eigen.h>
#include <geometry_msgs/PoseArray.h>
#include <geometry_msgs/PoseStamped.h>
#include <moveit_msgs/Grasp.h>
//#include <Eigen/Core>
//#include <Eigen/Geometry>

// Rviz
//#include <visualization_msgs/Marker.h>
//#include <visualization_msgs/MarkerArray.h>
#include <moveit_visual_tools/visual_tools.h>

// MoveIt
//#include <moveit/planning_scene_monitor/planning_scene_monitor.h>
#include <moveit/robot_state/robot_state.h>
#include <moveit/kinematics_plugin_loader/kinematics_plugin_loader.h>

// C++
#include <boost/thread.hpp>
#include <math.h>
#define _USE_MATH_DEFINES

namespace moveit_simple_grasps
{

// Struct for passing parameters to threads, for cleaner code
struct IkThreadStruct
{
  IkThreadStruct(
    std::vector<moveit_msgs::Grasp> &possible_grasps, // the input
    std::vector<moveit_msgs::Grasp> &filtered_grasps, // the result
    std::vector<trajectory_msgs::JointTrajectoryPoint> &ik_solutions, // the resulting solutions for each filtered grasp
    Eigen::Affine3d &link_transform,
    int grasps_id_start,
    int grasps_id_end,
    kinematics::KinematicsBaseConstPtr kin_solver,
    double timeout,
    boost::mutex *lock,
    int thread_id)
    : possible_grasps_(possible_grasps),
      filtered_grasps_(filtered_grasps),
      ik_solutions_(ik_solutions),
      link_transform_(link_transform),
      grasps_id_start_(grasps_id_start),
      grasps_id_end_(grasps_id_end),
      kin_solver_(kin_solver),
      timeout_(timeout),
      lock_(lock),
      thread_id_(thread_id)
  {
  }
  std::vector<moveit_msgs::Grasp> &possible_grasps_;
  std::vector<moveit_msgs::Grasp> &filtered_grasps_;
  std::vector<trajectory_msgs::JointTrajectoryPoint> &ik_solutions_;
  Eigen::Affine3d link_transform_;
  int grasps_id_start_;
  int grasps_id_end_;
  kinematics::KinematicsBaseConstPtr kin_solver_;
  double timeout_;
  boost::mutex *lock_;
  int thread_id_;
};


// Class
class GraspFilter
{
private:
  // State of robot
  robot_state::RobotState robot_state_;

  // Parameters from goal
  const std::string planning_group_;

  // threaded kinematic solvers
  std::vector<kinematics::KinematicsBaseConstPtr> kin_solvers_;

  // whether to publish grasp info to rviz
  bool rviz_verbose_;

  // class for publishing stuff to rviz
  moveit_visual_tools::VisualToolsPtr visual_tools_;

public:

  // Constructor
  GraspFilter( robot_state::RobotState robot_state, bool rviz_verbose,
    moveit_visual_tools::VisualToolsPtr visual_tools, const std::string& planning_group );

  // Destructor
  ~GraspFilter();

  // Of an array of grasps, choose just one for use
  bool chooseBestGrasp( const std::vector<moveit_msgs::Grasp>& possible_grasps,
    moveit_msgs::Grasp& chosen );

  // Take the nth grasp from the array
  bool filterNthGrasp(std::vector<moveit_msgs::Grasp>& possible_grasps, int n);

  // Choose the 1st grasp that is kinematically feasible
  bool filterGrasps(std::vector<moveit_msgs::Grasp>& possible_grasps,
    std::vector<trajectory_msgs::JointTrajectoryPoint>& ik_solutions);

private:

  // Thread for checking part of the possible grasps list
  void filterGraspThread(IkThreadStruct ik_thread_struct);


}; // end of class

typedef boost::shared_ptr<GraspFilter> GraspFilterPtr;
typedef boost::shared_ptr<const GraspFilter> GraspFilterConstPtr;

} // namespace

#endif
