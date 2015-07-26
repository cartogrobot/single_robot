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
    RobotMapPoint(double x, double y, const Angle & normal);
    
    // Constructs MapPoint from local polar coordinates and MapPoint for their origin
    RobotMapPoint(const PolarCoordinates & polarCoords, const RobotMapPoint & location);
    
    // Accessor for x coordinate
    double getX();
    
    // Accessor for y coordinate
    double getY();
    
    // Accessor for normal angle
    const Angle & getNormal();
    
private:
    // Cartesian coordinates
    double _x, _y;
    
    // Angle representing normal vector
    Angle _normal;
};


// World map of RobotMapPoints organized into grid sectors via a unordered_map
class RobotMap {
public:
    // Constructs map with the given scale
    RobotMap(double scale);
    
    // Adds a RobotMapPoint to the map and places it in the corresponding grid sector
    addPoint(const RobotMapPoint & p);
    
private:
    // Hash map that sorts points into bins by general location
    std::unordered_map<pair<int,int>, vector<RobotMapPoint>> _grid;
    
    // Scaling factor that determines size of grid sectors
    double _scale;
};


// Takes local sensor readings and computes location on map
class VotingGrid {
public:
    // Compares local sensor data to known map and computes most likely location and orientation
    RobotMapPoint Evaluate(const vector<PolarCoordinates> & data, const RobotMap & map);
    
private:
    // Grid square size
    double _scale;
    
    // Error allowed between normals
    Angle _angleThreshold;
};