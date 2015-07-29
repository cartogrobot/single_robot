/*
 * localization_point_norm.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Mapping system for robot localization using the point w/normal method
*/

#include<cmath>
#include<unordered_map>
#include<pair>


// Polar coordinates for a point
struct PolarCoordinates {
    double _r;
    Angle _angle;
};


// Stores a point as cartesian coordinates and a normal vector as angle
class RobotMapPoint {
public:
    // Constructs MapPoint with the given parameters
    RobotMapPoint(double x, double y, const Angle & angle);
    
    // Constructs MapPoint from local polar coordinates and MapPoint for their origin
    RobotMapPoint(const PolarCoordinates & polarCoords, const RobotMapPoint & location);
    
    // Accessor for x coordinate
    double getX();
    
    // Accessor for y coordinate
    double getY();
    
    // Accessor for normal angle
    const Angle & getNormal();
    
    // Self modifying addition
    RobotMapPoint & RobotMapPoint::operator+=();  
    
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
}


// World map of RobotMapPoints organized into grid sectors via a unordered_map
class RobotMap {
public:
    // Constructs map with the given scale
    RobotMap(double hashScale, double voteScale, double voteErrorAngle, int angleDivisions);
    
    // Adds a RobotMapPoint to the map and places it in the corresponding grid sector
    void addPoint(const RobotMapPoint & p);
    
    // Compares local sensor data to known map and computes most likely location and orientation
    RobotMapPoint feasiblePose(const vector<PolarCoordinates> & data);
    
private:
    // Hash map that sorts points into bins by general location
    std::unordered_map<std::pair<int,int>, vector<RobotMapPoint>> _grid;
    
    // Scaling factor for hash grid sectors
    double _hashScale;
    
    // Amount to vary angles
    int _angleDivisions;
    
    // Scaling factor for voting grid sectors
    double _voteScale;
    
    // Error allowed between normals
    double _voteAngleError;
    
    // Robot location
    RobotMapPoint location;
};