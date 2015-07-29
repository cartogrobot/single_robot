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

// Self modifying addition
RobotMapPoint & RobotMapPoint::operator+=(const RobotMapPoint & other){
    this->_x += other._x;
    this->_y += other._y;
    
    return *this;
}

// Non modifying addition
RobotMapPoint operator+(const RobotMapPoint & p1, const RobotMapPoint & p2){
    return RobotMapPoint(p1) += p2;
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

// Retrieves map points nearby the robot location
vector<RobotMapPoints> getNearbyPoints(){
    // Find discrete location of the robot
    int x = std::floor(_location.getX() * _scale);
    int y = std::floor(_location.getY() * _scale);
    
    // Vector to store nearby points
    vector<RobotMapPoints> nearbyPoints;
    
    // Find all points in the current or adjacent locations
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            for(const RobotMapPoint & point : _grid[std::pair(x+i,y+j)]){
                nearbyPoints.push_back(point);
            }
        }
    }
    
    return nearbyPoints;
}

// Compares local sensor data to known map and computes most likely location and orientation
RobotMapPoint RobotMap::feasiblePose(const vector<PolarCoordinates> & data){
    // Track the current most 
    int bestX = 0, bestY = 0, bestAngle = 0, maxVotes = 0;
    
    // Hash table to store feasible pose votes
    unordered_map<VoteLocation,int> votes;
    
    for(int i=0; i<angleDivisions; i++){
        Angle delta((pi2())/angleDivisions * i);
        
        for(PolarCoordinates pc : data){
            // Add pi to invert the vector and add the current angle offset
            pc._angle += delta + pi();
            
            // Retrieve map points nearby the robots location
            vector<RobotMapPoint> nearbyPoints = getNearbyPoints();
            
            // Look at all nearby points
            for(RobotMapPoint point : nearbyPoints){
                // Compare normals to see if they are similar
                if(point._angle > pc._angle - _voteErrorAngle &&
                   point._angle > pc._angle + _voteErrorAngle){
                    // Add the vector to the map point to find position
                    point += pc;
                    
                    // Calculate discrete location in grid
                    VoteLocation vl;
                    vl._x = std::floor(point.getX() * _voteScale);
                    vl._y = std::floor(poin.getY() * _voteScale);
                    vl._delta = i;
                    
                    // Add a vote to this location
                    votes[vl] += 1
                    
                    // Record locations that lead in votes
                    if(votes[vl] > maxVotes){
                        bestX = vl._y;
                        topy = vl._y;
                        bestAngle = vl._delta;
                        maxVotes += 1;
                    }
                }
            }
        }
    }
    
    // Estimate position from vote location
    double x = (1 / scale) / 2 + bestX;
    double y = (1 / scale) / 2 + bestY;
    Angle orientation(pi2() / angleDivisions * (0.5 + bestAngle));
    
    // Return the most probable robot location
    return RobotMapPoint(x, y, orientation);
}