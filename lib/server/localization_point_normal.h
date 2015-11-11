/*
 * localization_point_norm.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Mapping system for robot localization using the point w/normal method
*/

#ifndef __LOCALIZATION_POINT_NORMAL_H
#define __LOCALIZATION_POINT_NORMAL_H

#include<cmath>
#include<unordered_map>
#include<utility>
#include<queue>
#include "angle.h"

// Polar coordinates for a point
struct PolarCoordinates {
    double _r;
    Angle _angle;
};


struct pairhash {
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const
  {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

// Stores a point as cartesian coordinates and a normal vector as angle
class RobotMapPoint {
public:
    
    // Defualt ctor
    RobotMapPoint();
    
    // Copy ctor
    RobotMapPoint(const RobotMapPoint & other);
    
    // Constructs MapPoint with the given parameters
    RobotMapPoint(double x, double y, const Angle & angle);
    
    // Constructs MapPoint from local polar coordinates and MapPoint for their origin
    RobotMapPoint(const PolarCoordinates & polarCoords);
    
    // Constructs MapPoint from local polar coordinates and MapPoint for their origin
    RobotMapPoint(const PolarCoordinates & polarCoords, const RobotMapPoint & location);
    
    // Accessor for x coordinate
    double getX() const;
    
    // Accessor for y coordinate
    double getY() const;
    
    // Accessor for normal angle
    const Angle & getAngle() const;
    
    // Self modifying addition
    RobotMapPoint & operator+=(const RobotMapPoint & other);  
    
private:
    // Cartesian coordinates
    double _x, _y;
    
    // Angle representing normal vector
    Angle _angle;
};

// Non modifying addition
RobotMapPoint operator+(const RobotMapPoint & p1, const RobotMapPoint & p2);


struct VoteLocation{
    int _x, _y, _delta;
};

bool operator==(const VoteLocation & v1, const VoteLocation & v2);

struct votehash {
public:
    std::size_t operator()(const VoteLocation &x) const
    {
        return std::hash<int>()(x._x) ^ std::hash<int>()(x._y) ^ std::hash<int>()(x._delta);
    }
};



// World map of RobotMapPoints organized into grid sectors via a unordered_map
class RobotMap {
public:
	// Default constructor, constructs with 1, 1, 3.14, 16
	RobotMap();
	
    // Constructs map with the given scale
    RobotMap(double hashScale, double voteScale,
             const Angle & voteErrorAngle, std::size_t angleDivisions);
    
    // Adds a RobotMapPoint to the map and places it in the corresponding grid sector
    void addPoint(const RobotMapPoint & p);
    
    // Compares local sensor data to known map and computes most likely location and orientation
    RobotMapPoint feasiblePose(const std::vector<PolarCoordinates> & data);
    
private:
    // Retrieves map points nearby the robot location
    std::vector<RobotMapPoint> getNearbyPoints(std::size_t range = 1);

    // Hash map that sorts points into bins by general location
    std::unordered_map<std::pair<int,int>, std::vector<RobotMapPoint>, pairhash> _grid;
    
    // Scaling factor for hash grid sectors
    double _hashScale;
    
    // Scaling factor for voting grid sectors
    double _voteScale;
    
    // Error allowed between normals
    double _voteAngleError;
    
    // Amount to vary angles
    std::size_t _angleDivisions;
    
    // Robot location
    RobotMapPoint _location;
};

#endif