#ifndef SAMPLE_H
#define SAMPLE_H

#include "ros/ros.h"
#include <atomic>
#include <mutex>
#include "squiggles.hpp"

// #include "spline.hpp"
// #include "physicalmodel/tankmodel.hpp"
// #include "physicalmodel/physicalmodel.hpp"

//ROS data types
#include "std_srvs/SetBool.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "tf/transform_datatypes.h"
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/PoseArray.h"
#include "sensor_msgs/LaserScan.h"
#include "geometry_msgs/Twist.h"
#include "visualization_msgs/MarkerArray.h"
#include "nav_msgs/OccupancyGrid.h"
#include "std_msgs/Header.h"
#include "nav_msgs/MapMetaData.h"
#include <nav_msgs/GetPlan.h>

//We include header of another class we are developing
#include "laserprocessing.h"
#include "pathplanning.h"

/*!
 *  \brief     Sample Class
 *  \details
 *  This class is used for communicating with the simulator environment using ROS using the provided libraries.
 *  It is designed to use laserprocessing to interpret laser data and imageprocessing to interpret image data.
 *  This information is used to generate an input for the control or the Turtlebot to follow the AR tag.
 *  \author    Ashton Powell
 *  \version   1.00
 *  \date      2024-XX-XX
 */
class Sample
{
public:
  /// @brief Constructor of the Sample class.
  ///
  /// Sets the default values of variables such as the robot position, the goals, the running_ boolean, etc.
  /// Requires the NodeHandle input to communicate with ROS.
  Sample(ros::NodeHandle nh);

  /// @brief Destructor of the Sample class.
  ///
  /// Deletes the object pointers for laserprocessing and imageprocessing classes.
  ~Sample();
  
  /// @brief seperate thread.
  ///
  /// The main processing thread that will run continously and utilise the data.
  /// When data needs to be combined then running a thread seperate to callback will guarantee data is processed.
  /// The data is then used to publish input to move the TurtleBot, causing new data to be generated on its updated position and perspective.
  void seperateThread();

  void laserCallback(const sensor_msgs::LaserScanConstPtr& msg);

  /// @brief Odometry Callback from the world reference of the TurtleBot
  ///
  /// @param [in|out] msg nav_msgs::OdometryConstPtr - The odometry message
  /// @note This function and the declaration are ROS specific
  // void odomCallback(const nav_msgs::OdometryConstPtr& msg);

  /// @brief Odometry Callback from the world reference of the TurtleBot
  ///
  /// @param [in|out] msg nav_msgs::OdometryConstPtr - The odometry message
  /// @note This function and the declaration are ROS specific
  void amclCallback(const geometry_msgs::PoseWithCovarianceStampedConstPtr& msg);

  /// @brief Odometry Callback from the world reference of the TurtleBot
  ///
  /// @param [in|out] msg nav_msgs::OdometryConstPtr - The odometry message
  /// @note This function and the declaration are ROS specific
  void pathCallback(const geometry_msgs::PointConstPtr& msg);

  void mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg);

  visualization_msgs::Marker createMarker(geometry_msgs::Point point, double r, double g, double b);

  /// @brief request service callback for starting and stopping the mission and Turtlebot's movement.
  ///
  /// @param [in] req The request, a boolean value where true means the mission is in progress and false stops the mission.
  /// @param [in] res The response, a boolean and string value indicating if starting the mission was successful.
  ///
  /// @return bool - Will return true to indicate the request succeeded.
  bool request(std_srvs::SetBool::Request  &req,
               std_srvs::SetBool::Response &res);

  /// @brief request service callback for starting and stopping the mission and Turtlebot's movement.
  ///
  /// @param [in] req The request, a boolean value where true means the mission is in progress and false stops the mission.
  /// @param [in] res The response, a boolean and string value indicating if starting the mission was successful.
  ///
  /// @return bool - Will return true to indicate the request succeeded.
  bool real(std_srvs::SetBool::Request  &req,
            std_srvs::SetBool::Response &res);

  /// @brief Getter for distance to be travelled to reach goal, updates as the platform moves to current goal.
  ///
  /// @param [in] goal a geometry_msgs::Point that is the current goal in x,y,z for the Turtlebot.
  /// @param [in] robot a geometry_msgs::Pose that is the current position and orientation of the Turtlebot.
  ///
  /// @return distance of the goal to the Turtlebot in a straight line [m].
  double DistanceToGoal(geometry_msgs::Point goal, geometry_msgs::Pose robot);

  /// @brief Getter for distance between consecutive goals
  ///
  /// @param [in] goal1 a geometry_msgs::Point that is the first goal in x,y,z for the Turtlebot.
  /// @param [in] goal2 a geometry_msgs::Point that is the second goal in x,y,z for the Turtlebot.
  ///
  /// @return distance between consecutive goals in a straight line [m].
  double DistanceBetweenGoals(geometry_msgs::Point goal1, geometry_msgs::Point goal2);

  /// @brief Gets the angle of the required turn to follow a straight line towards the next goal.
  ///
  /// @param [in] goal a geometry_msgs::Point that is the current goal in x,y,z for the TurtleBot.
  /// @param [in] robot a geometry_msgs::Pose that is the current position and orientation of the TurtleBot.
  ///
  /// @return a double value for the angle of the required turn of the TurtleBot [rad].
  double GetGoalAngle(geometry_msgs::Point goal, geometry_msgs::Pose robot);

  double fabs(double x);

  /// @brief Laser Callback from the laser sensor's reference
  ///
  /// @param [in|out] msg sensor_msgs::LaserScanConstPtr - the laser scan data
  /// @note This function and the declaration are ROS specific

  void GenerateSpline();

  visualization_msgs::MarkerArray CollectGoals(visualization_msgs::MarkerArray markerArray);

  double GetGoalOrientation(std::vector<geometry_msgs::Point> goals, geometry_msgs::Pose robot);

  geometry_msgs::Point FindLookaheadPoint(std::vector<geometry_msgs::Point> goals);

  double computeCurvature(geometry_msgs::Point goal, geometry_msgs::Pose robot);

  double SmoothVel(unsigned int idx);

  std::vector<geometry_msgs::Point> generateRandomGoals(PathPlanning pathPlanning);

  std::vector<geometry_msgs::Point> planBetweenTwoGoals(PathPlanning pathPlanning, geometry_msgs::Point st, geometry_msgs::Point en);
  
private:
  //! Node handle for communication
  ros::NodeHandle nh_;
  //! Driving command publisher
  ros::Publisher pubDrive_;
  //! Driving command publisher
  ros::Publisher pubVis_;
  //! Goal publisher 
  ros::Publisher goal_pub_;
  //! Laser scan subscriber, uses LaserCallback
  ros::Subscriber sub1_;
  //! Robot odometry subscriber, uses OdomCallback
  ros::Subscriber sub2_;
  //! Robot odometry subscriber, uses AmclCallback
  ros::Subscriber sub3_;
  //! Map subscribe
  ros::Subscriber sub4_;
  //! Mission service, starts and stops the mission
  ros::ServiceServer service1_;
  //! Mission service, starts and stops the mission
  ros::ServiceServer service2_;
  //! Make Plan
  ros::ServiceClient make_plan_;
  
  //! Pointer to Laser Object
  LaserProcessing* laserProcessingPtr_;

  PathPlanning* pathPlanningPtr_;

  //! Stores the laser data from the LIDAR scanner
  sensor_msgs::LaserScan laserData_;
  //! Mutex to lock laserData_
  std::mutex laserDataMtx_;
  //! Stores the position and orientation of the robot
  geometry_msgs::Pose robotPose_;
  //! Mutex to lock robotPose_
  std::mutex robotPoseMtx_;
  //! Stores the position and orientation of the robot
  geometry_msgs::Point pathData_;
  //! Mutex to lock robotPose_
  std::mutex pathDataMtx_;

  //! Flag for whether the car is moving and the mission is active
  std::atomic<bool> running_;
  //! Flag for whether the it in sim or real life
  std::atomic<bool> real_;
  //! Flag to prevent the terminal from being flooded with messages
  bool stateChange_;
  //! Stores a goal for the robot to move towards
  geometry_msgs::Point goal_;
  //! Stores a series of goals in the order they occur
  std::vector<geometry_msgs::Point> goals_;
  //! The offset between the reference of the TurtleBot and the reference of the laser scanner
  double SENSOR_OFFSET_ = 0.12;
  //! The stop distance to stop the following TurtleBot before it collides with the guiding TurtleBot
  // double STOP_DISTANCE_ = 0.34;
  double STOP_DISTANCE_ = 0.24;

  double GOAL_DISTANCE_ = 0.1;
  //! Boolean for stopping the TurtleBot when it becomes too close to the guiding TurtleBot
  double STEERING_SENS_ = 0.8;
  
  bool tooClose_;

  int trajMode_ = 1;

  int goalIdx_;

  int velIdx_;

  double time_;

  double DBL_MAX_ = 1.7976931348623157E+308;

  const double MAX_VEL = 0.26;   // in meters per second
  const double MAX_ACCEL = 0.43; // in meters per second per second
  const double MAX_JERK = 1.0;  // in meters per second per second per second

  const double ROBOT_WIDTH_ = 0.3;

  std::vector<squiggles::ProfilePoint> path_;

  //! Provides the unique ID of the markers
  unsigned int marker_counter_;

  int smoothVelIdx_;

  double poseError_;

  double lookahead_dist_ = 0.4;

  int minIdx_;

  std::mutex mapMtx_;
  double threshold_distance_;
  int map_width_;
  int map_height_;
  double map_resolution_;
  double map_origin_x_;
  double map_origin_y_;
  std::vector<int8_t> map_data_;
  double world_x_;
  double world_y_;
  std::vector<geometry_msgs::PoseStamped> unordered_goals_;
};

#endif // SAMPLE_H