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

RobotMapPoint::RobotMapPoint(): _x(0.0), _y(0.0), _angle(0.0) {}

// Constructs MapPoint with the given parameters
RobotMapPoint::RobotMapPoint(double x, double y, const Angle & angle): _x(x), _y(y), _angle(angle) {}

// Constructs MapPoint from local polar coordinates and MapPoint for their origin
RobotMapPoint::RobotMapPoint(const PolarCoordinates & polarCoords){
    // Correct for robot orientation
    _angle = polarCoords._angle;
    
    // Convert to cartesian and add vectors
    _x = std::cos(_angle) * polarCoords._r;
    _y = std::sin(_angle) * polarCoords._r;
}


// Constructs MapPoint from local polar coordinates and MapPoint for their origin
RobotMapPoint::RobotMapPoint(const PolarCoordinates & polarCoords, const RobotMapPoint & location){
    // Correct for robot orientation
    _angle = polarCoords._angle + location._angle;
    
    // Convert to cartesian and add vectors
    _x = std::cos(_angle) * polarCoords._r + location._x;
    _y = std::sin(_angle) * polarCoords._r + location._y;
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
const Angle & RobotMapPoint::getAngle() const{
    return _angle;
}


bool operator==(const VoteLocation & v1, const VoteLocation & v2){
    return (v1._x == v2._x) && (v1._y == v2._y) && (v1._delta == v2._delta);
}


/* 
 * RobotMap
 * World map of RobotMapPoints organized into grid sectors via a unordered_map
*/

// Constructs map with the given scale
RobotMap::RobotMap(double hashScale, double voteScale, double voteAngleError, int angleDivisions):
    _hashScale(hashScale), _voteScale(voteScale), _voteAngleError(voteAngleError), _angleDivisions(angleDivisions) {}
    
// Adds a RobotMapPoint to the map and places it in the corresponding grid sector
void RobotMap::addPoint(const RobotMapPoint & p){
    int x = std::floor(p.getX() * _hashScale);
    int y = std::floor(p.getY() * _hashScale);
    
    _grid[std::pair<int,int>(x,y)].push_back(p);
}

// Retrieves map points nearby the robot location
std::vector<RobotMapPoint> RobotMap::getNearbyPoints(int range){
    // Find discrete location of the robot
    int x = std::floor(_location.getX() * _hashScale);
    int y = std::floor(_location.getY() * _hashScale);
    
    // Vector to store nearby points
    std::vector<RobotMapPoint> nearbyPoints;
    
    // Find all points in the current or adjacent locations
    for(int i=(-1)*range; i<=range; i++){
        for(int j=(-1)*range; j<=range; j++){
            std::pair<int,int> pos(x+i,y+j);
            if(_grid.count(pos) != 0){
                for(const RobotMapPoint & point : _grid[pos]){
                    nearbyPoints.push_back(point);
                }
            }
        }
    }
    
    return nearbyPoints;
}

// Compares local sensor data to known map and computes most likely location and orientation
RobotMapPoint RobotMap::feasiblePose(const std::vector<PolarCoordinates> & data){
    // Most votes given
    int maxVotes = 0;

    // Hash table to store feasible pose votes
    std::unordered_map<VoteLocation,int, votehash> votes;
    
    // Best vote locations
    std::queue<VoteLocation> bestLocations;
    
    // Start angle 90 degrees back from last known angle
    Angle initAngle = _location.getAngle() - pi()/2;
    
    for(int i=0; i<_angleDivisions; i++){
        // Increment test angle
        Angle testAngle(initAngle + (pi() / _angleDivisions) * i);
        
        for(PolarCoordinates pc : data){
            // Add pi to invert the vector and add the current angle offset
            pc._angle += testAngle + pi();
            
            // Retrieve map points nearby the robots location
            std::vector<RobotMapPoint> nearbyPoints = getNearbyPoints();
            
            // Look at all nearby points
            for(RobotMapPoint point : nearbyPoints){
                // Compare normals to see if they are similar
                if(point.getAngle() > pc._angle - _voteAngleError &&
                   point.getAngle() > pc._angle + _voteAngleError){
                    // Add the vector to the map point to find position
                    point += pc;
                    
                    // Calculate discrete location in grid
                    VoteLocation vl;
                    vl._x = std::floor(point.getX() * _voteScale);
                    vl._y = std::floor(point.getY() * _voteScale);
                    vl._delta = i;
                    
                    // Add a vote to this location
                    votes[vl] += 1;
                    
                    // Record locations that lead in votes
                    if(votes[vl] >= maxVotes){
                        // Add new top location to list
                        bestLocations.push(vl);
                        
                        // Track max votes
                        maxVotes = votes[vl];
                        
                        // Trim off to keep just top 10
                        if(bestLocations.size() > 10){
                            bestLocations.pop();
                        }
                    }
                }
            }
        }
    }
    
    // Estimate position from vote location
    double x = (bestLocations.back()._x + 0.5) * (1 / _voteScale);
    double y = (bestLocations.back()._y + 0.5) * (1 / _voteScale);
    Angle orientation = initAngle + (0.5 + bestLocations.back()._delta) * (pi() / _angleDivisions);
    
    // Return the most probable robot location
    return RobotMapPoint(x, y, orientation);
}