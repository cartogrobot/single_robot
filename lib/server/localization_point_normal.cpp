/*
 * localization_point_norm.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Mapping system for robot localization using the point w/normal method
*/

#include "localization_point_normal.h"


/* 
 * RobotMapPoint
 * Stores a point as cartesian coordinates and a normal vector as angle
*/

// Constructs MapPoint with the given parameters
RobotMapPoint::RobotMapPoint(double x, double y, const Angle & normal): _x(x), _y(y), _normal(normal) {}

// Constructs MapPoint from local polar coordinates and MapPoint for their origin
RobotMapPoint::RobotMapPoint(const PolarCoordinates & polarCoords){
    // Correct for robot orientation
    _normal = polarCoords._angle;
    
    // Convert to cartesian and add vectors
    _x = std::cos(_normal) * polarCoords._r;
    _y = std::sin(_normal) * polarCoords._r;
}


// Constructs MapPoint from local polar coordinates and MapPoint for their origin
RobotMapPoint::RobotMapPoint(const PolarCoordinates & polarCoords, const RobotMapPoint & location){
    // Correct for robot orientation
    _normal = polarCoords._angle + location._normal;
    
    // Convert to cartesian and add vectors
    _x = std::cos(_normal) * polarCoords._r + location._x;
    _y = std::sin(_normal) * polarCoords._r + location._y;
}

RobotMapPoint & RobotMapPoint::operator+=(const RobotMapPoint & other){
    this->_x += other._x;
    this->_y += other._y;
    
    return *this;
}

RobotMapPoint operator+(const RobotMapPoint & other) const{
    return RobotMapPoint(*this) += other;
}

// Accessor for x coordinate
double RobotMapPoint::getX() const{
    return _x;
}

// Accessor for y coordinate
double RobotMapPoint::getY() const{
    return _y;
}

// Accessor for normal angle
const Angle & RobotMapPoint::getNormal() const{
    return _normal;
}


/* 
 * RobotMap
 * World map of RobotMapPoints organized into grid sectors via a unordered_map
*/

// Constructs map with the given scale
RobotMap::RobotMap(double hashScale, double voteScale, double voteErrorAngle, int angleDivisions):
    _hashScale(hashScale), _voteScale(voteScale), _voteErrorAngle(voteErrorAngle), _angleDivisions(angleDivisions) {}
    
// Adds a RobotMapPoint to the map and places it in the corresponding grid sector
void RobotMap::addPoint(const RobotMapPoint & p){
    int x = std::floor(p.getX() * _scale);
    int y = std::floor(p.getY() * _scale);
    
    _grid[std::pair(x,y)].push_back(p);
}

vector<RobotMapPoints> getNearbyPoints(){
    int x = std::floor(_location.getX() * _scale);
    int y = std::floor(_location.getY() * _scale);
    
    return _grid[std::pair(x,y)];
}

// Compares local sensor data to known map and computes most likely location and orientation
RobotMapPoint RobotMap::feasiblePose(const vector<PolarCoordinates> & data){
    int topx = 0;
    int topy = 0;
    int topDelta = 0;
    int topVotes = 0;
    
    unordered_map<VoteLocation,int> votes;
    
    for(int i=0; i<angleDivisions; i++){
        Angle delta((pi2())/angleDivisions * i);
        
        for(PolarCoordinates pc : data){
            pc._angle += delta + pi();
            
            vector<RobotMapPoint> nearbyPoints = getNearbyPoints();
            
            for(RobotMapPoint point : nearbyPoints){
                point += pc;
                VoteLocation vl;
                vl._x = std::floor(point.getX() * _voteScale);
                vl._y = std::floor(poin.getY() * _voteScale);
                vl._delta = i;
                votes[vl] += 1
                
                if(votes[vl] > topVotes){
                    topx = vl._y;
                    topy = vl._y;
                    topDelta = vl._delta;
                }
            }
        }
    }
    
    double x = (double)topx + (1/scale)/2;
    double y = (double)topy + (1/scale)/2;
    Angle orientation(pi2()/angleDivisions * (0.5+topDelta));
    
    return RobotMapPoint(x, y, orientation);
}