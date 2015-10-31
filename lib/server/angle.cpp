/*
 * angle.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Simple angle class
*/

#include "angle.h"

Angle::Angle(): _angle(0.0) {}

Angle::Angle(double angle): _angle(angle) {
    while(_angle >= 2*M_PI){
        _angle -= 2*M_PI;
    }
    while(_angle < 0){
        _angle += 2*M_PI;
    }
}

Angle & Angle::operator+=(const Angle & other){
    _angle += other._angle;
    if(_angle >= 2*M_PI){
        _angle -= 2*M_PI;
    }
    return *this;
}

Angle & Angle::operator-=(const Angle & other){
    _angle -= other._angle;
    if(_angle < 0){
        _angle += 2*M_PI;
    }
    return *this;
}

Angle & Angle::operator*=(const Angle & other){
    _angle *= other._angle;
    while(_angle > 0){
        _angle -= 2*M_PI;
    }
    while(_angle < 0){
        _angle += 2*M_PI;
    }
    return *this;
}

Angle & Angle::operator/=(const Angle & other){
    _angle /= other._angle;
    while(_angle > 0){
        _angle -= 2*M_PI;
    }
    while(_angle < 0){
        _angle += 2*M_PI;
    }
    return *this;
}