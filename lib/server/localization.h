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
    RobotMapPoint(double x, double y, double t);
    RobotMapPoint(double r, double t, const RobotMapPoint & location);
    double getX();
    double getY();
    double getTheta();
private:
    double _x, _y, _t;   // Coordinates and normal vector angle
};

//
class RobotMap {
public:
    RobotMap(double scale);
    
    addPoint(const RobotMapPoint & p);
    
private:
    // Hash map that sorts points into bins by general location
    std::unordered_map<pair<int,int>, vector<RobotMapPoint>> _grid;
    double _scale;
};

typedef pair<double,double> polarCoords;

// Takes local sensor readings and computes location on map
class VotingGrid {
public:
    // Compares local sensor data to known map and computes most likely location and orientation
    RobotMapPoint Evaluate(const vector<pair<double,double>> & data, const RobotMap & map);
private:
    double _scale;  // Grid square size
    double _angleThreshold; // Error allowed between normals
};