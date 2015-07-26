/*
 * localization.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Mapping system for robot localization
*/

#include<cmath>
#include<unordered_map>
#include<pair>

// Stores a point as cartesian coordinates and normal vector as angle
class RobotMapPoint {
public:
    RobotMapPoint(double x, double y, const Angle & t);
    RobotMapPoint(double r, const Angle & t, const RobotMapPoint & location);
    
    double getX();
    double getY();
    const Angle & getNormal();
    
private:
    // Cartesian coordinates
    double _x, _y;
    
    // Angle representing normal vector
    Angle _t;
};


// World map of RobotMapPoints organized into grid sectors via a unordered_map
class RobotMap {
public:
    RobotMap(double scale);
    
    addPoint(const RobotMapPoint & p);
    
private:
    // Hash map that sorts points into bins by general location
    std::unordered_map<pair<int,int>, vector<RobotMapPoint>> _grid;
    
    // Scaling factor that determines size of grid sectors
    double _scale;
};


// Polar coordinates for a point
struct PolarCoords {
    double r;
    Angle t;
};


// Takes local sensor readings and computes location on map
class VotingGrid {
public:
    // Compares local sensor data to known map and computes most likely location and orientation
    RobotMapPoint Evaluate(const vector<PolarCoords> & data, const RobotMap & map);
    
private:
    // Grid square size
    double _scale;
    
    // Error allowed between normals
    Angle _angleThreshold;
};