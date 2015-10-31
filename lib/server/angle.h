/*
 * angle.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Simple angle class
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif

#ifndef __ANGLE_H
#define __ANGLE_H

#include<cmath>

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

#endif