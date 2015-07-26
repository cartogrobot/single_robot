/*
 * localization.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Mapping system for robot localization
*/

#include "map.h"

MapPoint::MapPoint(double x, double y, const Angle & t): _x(x), _y(y), _t(t) {}

MapPoint::MapPoint(double r, const Angle & t, const MapPoint & location){
    // Correct for robot orientation
    _t = t + location._t;
    
    // Convert to cartesian and add vectors
    _x = std::cos(_t) * r + location._x;
    _y = std::sin(_t) * r + location._y;
}

double MapPoint::getX(){
    return _x;
}

double MapPoint::getY(){
    return _y;
}

const Angle & MapPoint::getNormal(){
    return _t;
}


RobotMap::RobotMap(double scale): _scale(scale) {}
    
RobotMap::addPoint(const RobotMapPoint & p){
    int x = std::floor(p.getX() * _scale);
    int y = std::floor(p.getY() * _scale);
    
    _grid[pair(x,y)].push_back(p);
}


// Compares local sensor data to known map and computes most likely location and orientation
RobotMapPoint VotingGrid::Evaluate(const vector<PolarCoords> & data, const MapPoint & location,
                                   const RobotMap & map){
    
}