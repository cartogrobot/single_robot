/*
 * angle.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Simple angle class
*/

Angle::Angle(double angle): _angle(angle) {
    while(_angle >= pi2()){
        _angle -= pi2();
    }
    while(_angle < 0){
        _angle += pi2();
    }
}

Angle & Angle::operator+=(const Angle & other){
    _angle += other._angle;
    if(_angle >= pi2()){
        _angle -= pi2();
    }
    return *this;
}

Angle & Angle::operator-=(const Angle & other){
    _angle -= other._angle;
    if(_angle < 0){
        _angle += pi2();
    }
    return *this;
}

Angle & Angle::operator*=(const Angle & other){
    _angle *= other._angle;
    while(_angle > 0){
        _angle -= pi2();
    }
    while(_angle < 0){
        _angle += pi2();
    }
    return *this;
}

Angle & Angle::operator/=(const Angle & other){
    _angle /= other._angle;
    while(_angle > 0){
        _angle -= pi2();
    }
    while(_angle < 0){
        _angle += pi2();
    }
    return *this;
}


Angle operator+(const Angle & a1, const Angle & a2){
    return Angle(a1) += a2;
}

Angle operator-(const Angle & a1, const Angle & a2){
    return Angle(a1) -= a2;
}