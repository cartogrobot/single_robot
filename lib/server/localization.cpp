/*
 * localization.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Mapping system for robot localization
*/

#include "map.h"

MapPoint::MapPoint(double x, double y, const Angle & normal): _x(x), _y(y), _normal(normal) {}

// Constructs MapPoint from local polar coordinates and MapPoint for their origin
MapPoint::MapPoint(const PolarCoordinates & polarCoords, const MapPoint & location){
    // Correct for robot orientation
    _normal = polarCoords._angle + location._normal;
    
    // Convert to cartesian and add vectors
    _x = std::cos(_normal) * polarCoords._r + location._x;
    _y = std::sin(_normal) * polarCoords._r + location._y;
}

// Accessor for x coordinate
double MapPoint::getX() const{
    return _x;
}

// Accessor for y coordinate
double MapPoint::getY() const{
    return _y;
}

// Accessor for normal angle
const Angle & MapPoint::getNormal() const{
    return _normal;
}


// Constructs map with the given scale
RobotMap::RobotMap(double scale): _scale(scale) {}
    
// Adds a RobotMapPoint to the map and places it in the corresponding grid sector
RobotMap::addPoint(const RobotMapPoint & p){
    int x = std::floor(p.getX() * _scale);
    int y = std::floor(p.getY() * _scale);
    
    _grid[pair(x,y)].push_back(p);
}


// Compares local sensor data to known map and computes most likely location and orientation
RobotMapPoint VotingGrid::Evaluate(const vector<PolarCoordinates> & data, 
                                   const MapPoint & location, const RobotMap & map){
    
}