/*
 * angle.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Simple angle class
*/

#include<cmath>

constexpr double pi() { return std::atan(-1); }
constexpr double pi2() { return std::atan(-1) * 2; }

// Stores an angle in radians between 0 and 2pi
class Angle {
public:
    Angle();
    Angle(double angle);
    
    // Self modifying arithmetic
    Angle & operator+=(const Angle & other);
    Angle & operator-=(const Angle & other);
    Angle & operator*=(const Angle & other);
    Angle & operator/=(const Angle & other);
    
    operator double() const{
        return _angle;
    }
    
private:

    double _angle;
};